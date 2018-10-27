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
#include "DriverKit.h"
#include "Utils.h"
#include "MainLoop.h"
#include "Credentials.h"

//#define APP_PRIORITY seL4_MaxPrio
#define APP_IMAGE_NAME "app"




static InitContext context = { 0 };


static Process initProcess = {0};



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


    error = !DriverKitInit(&context);
    ZF_LOGF_IFERR(error, "Failed to init DriverKit.\n");
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


/* BEGIN PROCESS */


    Process *testProcess = ProcessAlloc();
    error = ProcessStart(&context, testProcess,"TestSysCalls", context.ep_cap_path, &initProcess, APP_PRIORITY );
    if (error == 0)
    {
        ProcessTableAppend(testProcess);
    }
    else 
    {
	printf("Error spawning  TestSysCalls\n");
    }


    Process *process2 = ProcessAlloc();
    error = ProcessStart(&context,  process2,"shell", context.ep_cap_path, &initProcess, APP_PRIORITY );
    if(error == 0)
    {
        ProcessTableAppend(process2);
    }

//
    printf("Init : Got %i processes \n" , ProcessTableGetCount() );

    processLoop( &context,ep_object.cptr);//ep_cap_path);


    return 0;
}
