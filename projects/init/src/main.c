#include <stdio.h>
#include <cpio/cpio.h>

#include <sel4platsupport/bootinfo.h>

#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>

#include <simple/simple.h>
#include <simple-default/simple-default.h>

#include <sel4utils/vspace.h>


#include <vka/object_capops.h> // vka_mint_object


#include <SysCallNum.h>
#include "Bootstrap.h"
#include "ProcessDef.h"
#include "ProcessTable.h"
#include "Timer.h"

#include "Utils.h"
#include "MainLoop.h"

//#define APP_PRIORITY seL4_MaxPrio
#define APP_IMAGE_NAME "app"

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER   (1 << 1)
#define IRQ_BADGE_NETWORK (1 << 0)



static InitContext context = { 0 };


static Process initProcess = {0};



static void processTimer(seL4_Word sender_badge)
{
	sel4platsupport_handle_timer_irq(&context.timer, sender_badge);


	Timer*firedTimer = NULL;


 	TimerTick remain = TimerWheelGetTimeout(&context.timersWheel);
//	printf("processTimer remains %lu \n",remain); 
	// handle expired timers
	int count = 0;

	while (( firedTimer = TimerWheelGetFiredTimers(  &context.timersWheel ) ) != NULL )
//	while ((firedTimer  = TimerWheelGetFiredTimers( &context.timersWheel)) )
	{

		Process* attachedProcess = TimerGetUserContext( firedTimer);
		assert(attachedProcess);

//		printf("Got a fired timer to process %i!\n" , attachedProcess->_pid);

		seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 1);
		seL4_SetMR(0, __NR_nanosleep);

		seL4_Send(attachedProcess->reply , tag);

		cnode_delete(&context,attachedProcess->reply);

		assert(TimerWheelRemoveTimer( &context.timersWheel , firedTimer ) );

		TimerRelease(firedTimer);
		attachedProcess->reply = 0;
		count++;
	}
/*
	if(count)
	{
	    printf("Processed %i timers \n" , count);
	}
*/
}


static void processLoop( seL4_CPtr epPtr )
{
       int error = 0;
       while(1)
       {
//	    TimerTick remain = TimerWheelGetTimeout(&context.timersWheel);
//	    error = ltimer_set_timeout(&context.timer.ltimer, NS_IN_MS*remain, TIMEOUT_PERIODIC);
//   	    printf("Error %i\n",error);
//	    assert(error == 0);

	    uint64_t startTimeNS;
            ltimer_get_time(&context.timer.ltimer, &startTimeNS);
	    
            seL4_Word sender_badge = 0;
            seL4_MessageInfo_t message;
            seL4_Word label;
//////////
            message = seL4_Recv(epPtr, &sender_badge);
//////////

	    uint64_t endTimeNS;
            ltimer_get_time(&context.timer.ltimer, &endTimeNS);

            const uint64_t timeSpentNS = endTimeNS - startTimeNS;

            TimerWheelStep(&context.timersWheel, timeSpentNS/1000000);

            TimerTick remain = TimerWheelGetTimeout(&context.timersWheel);
            if(remain <UINT64_MAX && remain != 0)
            {
//                printf("timersWheel update to %lu\n" , remain);

                int error = UpdateTimeout(&context, NS_IN_MS*remain);
                assert(error == 0);
            }
  /*          else 
            {
                printf("UINT64_MAX in timersWheel\n");
            }
*/


/////////
            label = seL4_MessageInfo_get_label(message);

            if(sender_badge & IRQ_EP_BADGE)
            {
                processTimer(sender_badge);
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

                processSyscall(&context,senderProcess , message , sender_badge );
            } // else if(label == seL4_NoFault)
            else
            {
                printf("ProcessLoop : other msg \n");
            }

/*
	    uint64_t endTimeNS;
            ltimer_get_time(&context.timer.ltimer, &endTimeNS);

	    const uint64_t timeSpentNS = endTimeNS - startTimeNS;

	    TimerWheelStep(&context.timersWheel, timeSpentNS/1000000);
    	    TimerTick remain = TimerWheelGetTimeout(&context.timersWheel);
	    if(remain <UINT64_MAX && remain != 0)
	    {
	        printf("timersWheel update to %lu\n" , remain);
                int error = ltimer_set_timeout(&context.timer.ltimer, NS_IN_MS*remain, TIMEOUT_RELATIVE); //TIMEOUT_PERIODIC);
                assert(error == 0);
	    }
	    else 
	    {
		printf("UINT64_MAX in timersWheel\n");
	    }
*/

        } // end while(1)
}



int main(void)
{


    memset(&context , 0 , sizeof(InitContext) );

    printf("init started\n");

    zf_log_set_tag_prefix("init:");

    UNUSED int error = 0;

    context.info = platsupport_get_bootinfo();
    ZF_LOGF_IF(context.info == NULL, "Failed to get bootinfo.");

    bootstrapSystem( &context);

    ZF_LOGF_IFERR(error, "Failed to bootstrap system.\n");

/* Processes table & init */

    ProcessInit(&initProcess);

    initProcess._pid = 1;

    ProcessTableInit();

    ProcessTableAppend(&initProcess);


 /* create an endpoint. */
    vka_object_t ep_object = {0};
    error = vka_alloc_endpoint(&context.vka, &ep_object);
    ZF_LOGF_IFERR(error, "Failed to allocate new endpoint object.\n");

    vka_cspace_make_path(&context.vka, ep_object.cptr, &context.ep_cap_path);

/* Timer Init */

    error = vka_alloc_notification(&context.vka, &context.ntfn_object);
    assert(error == 0);


    cspacepath_t notification_path;
    error = seL4_TCB_BindNotification(seL4_CapInitThreadTCB, context.ntfn_object.cptr);
    ZF_LOGF_IFERR(error, "Unable to BindNotification.\n");

    vka_cspace_make_path( &context.vka, context.ntfn_object.cptr, &notification_path);

    error  = TimerDriverInit(&context ,notification_path.capPtr);

    ZF_LOGF_IFERR(error, "Unable to initialize timer.\n");


    error = !TimersWheelInit(&context.timersWheel); // TimersWheelInit returns 1 on sucess -> negate 
    ZF_LOGF_IFERR(error, "Unable to initialize Timers Wheel.\n");


   uint64_t timerResolution = 0;;
   error = ltimer_get_resolution(&context.timer.ltimer , &timerResolution);

   printf("Timer resolution is %lu (error %i)\n" ,timerResolution , error);

//   error = ltimer_set_timeout(&context.timer.ltimer, NS_IN_MS*TIME_TO_WAIT_MS, TIMEOUT_RELATIVE); //TIMEOUT_PERIODIC);
//   assert(error == 0);

/* BEGIN PROCESS */

/*
    Process *process1 = ProcessAlloc();
    error = startProcess(  process1,APP_IMAGE_NAME, ep_cap_path );
    if (error == 0)
    {
        ProcessTableAppend(process1);
    }
*/

    Process *process2 = ProcessAlloc();
    error = startProcess(&context,  process2,"shell", context.ep_cap_path, &initProcess, APP_PRIORITY );
    if(error == 0)
    {
        ProcessTableAppend(process2);
    }

//
    printf("Init : Got %i processes \n" , ProcessTableGetCount() );

    processLoop(ep_object.cptr);//ep_cap_path);


    return 0;
}
