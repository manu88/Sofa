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

#define APP_PRIORITY seL4_MaxPrio
#define APP_IMAGE_NAME "app"

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER   (1 << 1)
#define IRQ_BADGE_NETWORK (1 << 0)

static InitContext context = { 0 };


static Process initProcess = {0};

static pid_t getNextPid()
{
	static pid_t accum = 2; // 1 is reserved for init
	return accum++;
}

static int startProcess( Process* process,const char* imageName, cspacepath_t ep_cap_path )
{

    UNUSED int error = 0;

    sel4utils_process_config_t config = process_config_default_simple( &context.simple, imageName, APP_PRIORITY);

    error = sel4utils_configure_process_custom( &process->_process , &context.vka , &context.vspace, config);

    ZF_LOGF_IFERR(error, "Failed to spawn a new thread.\n"
                  "\tsel4utils_configure_process expands an ELF file into our VSpace.\n"
                  "\tBe sure you've properly configured a VSpace manager using sel4utils_bootstrap_vspace_with_bootinfo.\n"
                  "\tBe sure you've passed the correct component name for the new thread!\n");



    process->_pid = getNextPid();

    seL4_CPtr process_ep_cap = 0;

    process_ep_cap = sel4utils_mint_cap_to_process(&process->_process, ep_cap_path,
                                               seL4_AllRights, process->_pid);

    ZF_LOGF_IF(process_ep_cap == 0, "Failed to mint a badged copy of the IPC endpoint into the new thread's CSpace.\n"
               "\tsel4utils_mint_cap_to_process takes a cspacepath_t: double check what you passed.\n");


    process_config_fault_cptr(config, ep_cap_path.capPtr);

    seL4_Word argc = 1;
    char string_args[argc][WORD_STRING_SIZE];
    char* argv[argc];
    sel4utils_create_word_args(string_args, argv, argc ,process_ep_cap);

    printf("init : Start child \n");
    error = sel4utils_spawn_process_v(&process->_process , &context.vka , &context.vspace , argc, (char**) &argv , 1);
    ZF_LOGF_IFERR(error, "Failed to spawn and start the new thread.\n"
                  "\tVerify: the new thread is being executed in the root thread's VSpace.\n"
                  "\tIn this case, the CSpaces are different, but the VSpaces are the same.\n"
                  "\tDouble check your vspace_t argument.\n");

     printf("init : Did start child pid %i\n" , process->_pid);


     process->_parent = &initProcess;
     return error;
}


static void processSyscall(Process *senderProcess, seL4_MessageInfo_t message, seL4_Word badge)
{
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

        sel4utils_destroy_process( &senderProcess->_process, &context.vka);
        ProcessRelease(senderProcess);

        printf("Init : Got %i processes \n" , ProcessTableGetCount() );
    }
    else if (msg == __NR_nanosleep)
    {
        seL4_Word millisToWait = seL4_GetMR(1);
        printf("Process %i request to sleep %li ms\n" , senderProcess->_pid , millisToWait);
    	senderProcess->_timer = TimerAlloc( senderProcess,1/*oneShot*/);
	assert(senderProcess->_timer);
	assert(TimerWheelAddTimer(&context.timersWheel , senderProcess->_timer , millisToWait));
    }
    else
    {
        printf("Received OTHER IPC MESSAGE\n");
    }

}


static void processTimer(seL4_Word sender_badge)
{
//	printf("SHOULD PROCESS TIMER !\n");


	sel4platsupport_handle_timer_irq(&context.timer, sender_badge);

	TimerWheelStep(&context.timersWheel, 1/*MS*/);

	Timer*firedTimer = TimerWheelGetFiredTimers( &context.timersWheel);

	if( firedTimer)
	{
		printf("Got a fired timer to process!\n");
	}
	
}
static void processLoop( seL4_CPtr epPtr )
{
       int error = 0;
       while(1)
       {
	    /*
	    uint64_t tm = (uint64_t)TimerWheelGetTimeout(&context.timersWheel);
            printf("init : Main timeout  %lu \n", tm);
            error = ltimer_set_timeout(&context.timer.ltimer, tm, TIMEOUT_ABSOLUTE);
	    if (error != 0)
	    {
		printf("ltimer_set_timeout error %i\n",error);
	    }
            assert(error == 0);
	    */
            seL4_Word sender_badge = 0;
            seL4_MessageInfo_t message;
	    seL4_Word label;

//	    printf("Wait...\n");
//	    message = seL4_Wait(epPtr, &sender_badge);
            message = seL4_Recv(epPtr, &sender_badge);
	    label = seL4_MessageInfo_get_label(message);
		
	    if(sender_badge & IRQ_EP_BADGE)
	    {
		processTimer(sender_badge);
	    }
	    else if(label == seL4_NoFault) 
	    {

                Process* senderProcess =  ProcessTableGetByPID( sender_badge);
		
		if(!senderProcess)
		{
		    printf("Init : no sender process for badge %li\n", sender_badge);
		    continue;
		}

		processSyscall(senderProcess , message , sender_badge );
	    } // else if(label == seL4_NoFault)
	    else 
	    {
		printf("ProcessLoop : other msg \n");
	    }
        }

}



int main(void)
{

    cspacepath_t ep_cap_path;

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

    vka_cspace_make_path(&context.vka, ep_object.cptr, &ep_cap_path);

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

   error = ltimer_set_timeout(&context.timer.ltimer, NS_IN_MS, TIMEOUT_PERIODIC);
   assert(error == 0);

// Test timer
/*
    Timer timer1;
    const TimerTick timeout = 10000; // ms
    assert( TimerInit(&timer1, NULL, 0) );
    assert(TimerWheelAddTimer(&context.timersWheel, &timer1, timeout) );

    uint64_t time;
    ltimer_get_time(&context.timer.ltimer, &time);
    printf("Start Time:  %lu \n" , time);

    Timer* firedTimer = NULL;

    while(firedTimer == NULL)
    {
        uint64_t tm = (uint64_t)TimerWheelGetTimeout(&context.timersWheel);
	printf("Wait for %lu \n", tm);
	error = ltimer_set_timeout(&context.timer.ltimer, tm*NS_IN_MS, TIMEOUT_PERIODIC);
        assert(error == 0);


        seL4_Word badgeTimer;
	uint64_t startTimeNS;
        ltimer_get_time(&context.timer.ltimer, &startTimeNS);

	seL4_Wait(ep_object.cptr, &badgeTimer);

	if(badgeTimer & IRQ_EP_BADGE)
	{
		// TODO Check other irqs
	 	sel4platsupport_handle_timer_irq(&context.timer, badgeTimer);

        	uint64_t endTimeNS;
        	ltimer_get_time(&context.timer.ltimer, &endTimeNS);

		const uint64_t diffNS = endTimeNS - startTimeNS;

		printf("TICK %lu \n" , diffNS/1000000);
  		TimerWheelStep(&context.timersWheel, diffNS/1000000);
	}

	firedTimer = TimerWheelGetFiredTimers( &context.timersWheel);
    }


    // get the current time 
    ltimer_get_time(&context.timer.ltimer, &time);
    printf("Did wait for %lu \n" , time);

*/

/* BEGIN PROCESS */

    Process *process1 = ProcessAlloc();
    error = startProcess(  process1,APP_IMAGE_NAME, ep_cap_path );
    if (error == 0)
    {
	ProcessTableAppend(process1);
    }

/*
    Process *process2 = ProcessAlloc();
    error = startProcess(  process2,APP_IMAGE_NAME, ep_cap_path );
    if(error == 0)
    {
        ProcessTableAppend(process2);
    }
*/
    printf("Init : Got %i processes \n" , ProcessTableGetCount() );

    processLoop(ep_object.cptr);//ep_cap_path);


    return 0;
}
