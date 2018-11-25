/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainLoop.h"
#include "ProcessTable.h"
#include <SysCallNum.h>
#include "Utils.h"
#include "SysCalls.h"
#include "DriverKit.h"

/*
Jump call table, MUST BE ORDERED with respect to __SOFA_NR_* numbers idefined in libSysCall/SysCallNum.h
*/
static SysCallHandler callTable[] = 
{
	NULL, // index 0 is always NULL, no syscall can have this id.

	handle_read,
	handle_write,
	handle_open,
	handle_close,

	handle_nanosleep,
	handle_getpid,
	handle_getppid,
	handle_exit,
	handle_kill,
	handle_execve,
	handle_wait4,
	handle_setpriority,
	handle_getpriority,
	handle_lseek,

	handle_getTimeOfTheDay,
	handle_clock_getTime,

 	handle_getcwd,
	handle_chdir,
//	handle_getdents64,
	handle_fcntl,
	handle_mkdir,


};


static void processTimer(KernelTaskContext* context,seL4_Word sender_badge);
static void processSyscall(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message, seL4_Word badge);

static void processTimer(KernelTaskContext* context,seL4_Word sender_badge)
{
    sel4platsupport_handle_timer_irq(&context->timer, sender_badge);

    Timer*firedTimer = NULL;


    TimerTick remain = TimerWheelGetTimeout(&context->timersWheel);

    while (( firedTimer = TimerWheelGetFiredTimers(  &context->timersWheel ) ) != NULL )
    {
	printf("Got a timer to handle\n");
	TimerContext* timerCtx = TimerGetUserContext( firedTimer);
	assert(timerCtx);
	assert(timerCtx->process);

        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
        seL4_SetMR(0, __SOFA_NR_nanosleep);
	seL4_SetMR(1, 0); // sucess

        seL4_Send(timerCtx->reply , tag);

        cnode_delete(context,timerCtx->reply);

        assert(TimerWheelRemoveTimer( &context->timersWheel , &timerCtx->timer ) );

//        TimerRelease(firedTimer);
  //      attachedProcess->reply = 0;
	free(timerCtx);
    }
}

int UpdateTimeout(KernelTaskContext* context,uint64_t timeNS)
{
    return ltimer_set_timeout(&context->timer.ltimer, timeNS, TIMEOUT_RELATIVE); //TIMEOUT_PERIODIC);
}


static void processSyscall(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message, seL4_Word badge)
{

    assert(senderProcess);

    seL4_Word msg;
    msg = seL4_GetMR(0);
    int error = callTable[msg](context , senderProcess,message);

    assert(error == 0);
}



void processLoop(KernelTaskContext* context, seL4_CPtr epPtr  )
{
    int error = 0;
    while(1)
    {
        uint64_t startTimeNS;
        ltimer_get_time(&context->timer.ltimer, &startTimeNS);

        seL4_Word sender_badge = 0;
        seL4_MessageInfo_t message;
        seL4_Word label;

        message = seL4_Recv(epPtr, &sender_badge);


        uint64_t endTimeNS;
        ltimer_get_time(&context->timer.ltimer, &endTimeNS);

        const uint64_t timeSpentNS = endTimeNS - startTimeNS;

        TimerWheelStep(&context->timersWheel, timeSpentNS/1000000);

        TimerTick remain = TimerWheelGetTimeout(&context->timersWheel);
        if(remain <UINT64_MAX && remain != 0)
        {
            int error = UpdateTimeout(context, NS_IN_MS*remain);
            assert(error == 0);
        }

        label = seL4_MessageInfo_get_label(message);

        if(sender_badge & IRQ_EP_BADGE)
        {
	    
	    if (sender_badge & IRQ_BADGE_TIMER)
	    {
		 processTimer(context ,sender_badge);
   	    }
	    else 
	    {
	        IOBaseDevice *dev = DriverKitGetDeviceForBadge( sender_badge - IRQ_EP_BADGE );

   	        if (dev)
	        {
		    dev->HandleIRQ(dev , -1);
//		    continue;
	        }
	        else 
   	        {
			printf("NOT FOUND device for badge %lx\n" , sender_badge - IRQ_EP_BADGE);

	        }
	    }

	    processTimer(context ,sender_badge);
	}
        else if (label == seL4_VMFault)
        {
            printf("Init : VM Fault \n");
        }
        else if (label == seL4_NoFault)
        {
            Process* senderProcess =  ProcessTableGetByPID( sender_badge);

            if(!senderProcess)
            {
                printf("Init : no sender process for badge %li\n", sender_badge);
		assert(0);
                continue;
            }

            processSyscall(context,senderProcess , message , sender_badge );
        }
        else
        {
            printf("ProcessLoop : other msg \n");
        }
    } // end while(1)
}

