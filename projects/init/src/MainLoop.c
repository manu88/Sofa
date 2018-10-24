#include "MainLoop.h"
#include "ProcessTable.h"
#include <SysCallNum.h>
#include "Utils.h"



/*static*/ void processTimer(InitContext* context,seL4_Word sender_badge)
{
        sel4platsupport_handle_timer_irq(&context->timer, sender_badge);


        Timer*firedTimer = NULL;


        TimerTick remain = TimerWheelGetTimeout(&context->timersWheel);

        while (( firedTimer = TimerWheelGetFiredTimers(  &context->timersWheel ) ) != NULL )
        {

                Process* attachedProcess = TimerGetUserContext( firedTimer);
                assert(attachedProcess);

                seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 1);
                seL4_SetMR(0, __NR_nanosleep);

                seL4_Send(attachedProcess->reply , tag);

                cnode_delete(context,attachedProcess->reply);

                assert(TimerWheelRemoveTimer( &context->timersWheel , firedTimer ) );

                TimerRelease(firedTimer);
                attachedProcess->reply = 0;
        }
}

int UpdateTimeout(InitContext* context,uint64_t timeNS)
{
    return ltimer_set_timeout(&context->timer.ltimer, timeNS, TIMEOUT_RELATIVE); //TIMEOUT_PERIODIC);
}

void processSyscall(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message, seL4_Word badge)
{
	int error = 0;
    assert(senderProcess);

    seL4_Word msg;
    msg = seL4_GetMR(0);

    if (msg ==  __NR_getpid)
    {

        seL4_SetMR(1, senderProcess->_pid);
        seL4_Reply( message );
    }
    else if (msg == __NR_getppid)
    {
        seL4_SetMR(1, senderProcess->_parent->_pid );
        seL4_Reply( message );

    }
    else if (msg == __NR_exit)
    {
        printf("Got exit from %i\n", senderProcess->_pid);

        if(!ProcessTableRemove( senderProcess))
        {
            printf("Unable to remove process!\n");
        }

        sel4utils_destroy_process( &senderProcess->_process, &context->vka);
        ProcessRelease(senderProcess);

        printf("Init : Got %i processes \n" , ProcessTableGetCount() );
    }

else if (msg == __NR_kill)
    {
        seL4_Word pidToKill = seL4_GetMR(1);
        seL4_Word sigToSend = seL4_GetMR(2);
        
        printf("Received a request from %i to kill process %li with signal %li\n",senderProcess->_pid , pidToKill , sigToSend);
        seL4_SetMR(1, -ENOSYS ); // error for now
        seL4_Reply( message );
    }
    else if (msg == __NR_nanosleep)
    {
        seL4_Word millisToWait = seL4_GetMR(1);
        //printf("Process %i request to sleep %li ms\n" , senderProcess->_pid , millisToWait);

        // save the caller

        senderProcess->reply = get_free_slot(context);

        error = cnode_savecaller( context, senderProcess->reply );

        assert(error == 0);

        Timer* timer = TimerAlloc( senderProcess,1/*oneShot*/);
        assert(timer);
        assert(TimerWheelAddTimer(&context->timersWheel , timer , millisToWait));
        UpdateTimeout(context,millisToWait * NS_IN_MS);
    }
else if(msg ==  __NR_execve)
    {

        const int msgLen = seL4_MessageInfo_get_length(message);
        assert(msgLen > 0);
        char* filename = malloc(sizeof(char)*msgLen );/* msglen minus header + 1 byte for NULL byte*/
        assert(filename);
        
        for(int i=0;i<msgLen-1;++i)
        {
            filename[i] =  (char) seL4_GetMR(1+i);
        }

        filename[msgLen] = '0';

        printf("Init : Execve size %i filename '%s'\n", msgLen , filename);

        /**/
        Process *newProcess = ProcessAlloc();
        assert(newProcess);
        error = startProcess(context,  newProcess,filename, context->ep_cap_path ,senderProcess, APP_PRIORITY);
        if (error == 0)
        {
            error = !ProcessTableAppend(newProcess);
            assert(error == 0);
        }


        /**/
        free(filename);

        seL4_SetMR(1, newProcess->_pid/*  -ENOSYS*/ ); // return code is the new pid
        seL4_Reply( message );
    }
else if (msg == __NR_wait4)
    {
        const pid_t pidToWait = seL4_GetMR(1);
        const int options     = seL4_GetMR(2); 

        seL4_SetMR(1,-ENOSYS );
        seL4_Reply( message );
    }
    else
    {
        printf("Received OTHER IPC MESSAGE\n");
        assert(0);
    }
}


void processLoop(InitContext* context, seL4_CPtr epPtr )
{
int error = 0;
       while(1)
       {
            uint64_t startTimeNS;
            ltimer_get_time(&context->timer.ltimer, &startTimeNS);

            seL4_Word sender_badge = 0;
            seL4_MessageInfo_t message;
            seL4_Word label;
//////////
            message = seL4_Recv(epPtr, &sender_badge);
//////////

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
                processTimer(context ,sender_badge);
            }
            else if (label == seL4_VMFault)
            {
                printf("VM Fault \n");
            }

	else if (label == seL4_NoFault)
            {
                Process* senderProcess =  ProcessTableGetByPID( sender_badge);

                if(!senderProcess)
                {
                    printf("Init : no sender process for badge %li\n", sender_badge);
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
