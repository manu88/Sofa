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
#include <sel4platsupport/arch/io.h>

#include <SysCallNum.h>
#include "Bootstrap.h"
#include "ProcessDef.h"
#include "ProcessTable.h"
#include "Timer.h"
#include "DriverKit.h"
#include "Utils.h"
#include "MainLoop.h"
#include "Credentials.h"
#include "FileServer.h"
#include "CpioServer.h"
#include "DevServer.h"
#include "EGADriver.h"

//#define APP_PRIORITY seL4_MaxPrio
#define APP_IMAGE_NAME "app"




static InitContext context = { 0 };


static Process initProcess = {0};


// creates IRQHandler cap "handler" for IRQ "irq"
static void get_irqhandler_cap(int irq, cspacepath_t* handler)
{
    seL4_CPtr cap;
    // get a cspace slot
    UNUSED int err = vka_cspace_alloc(&context.vka, &cap);
    assert(err == 0);

    // convert allocated cptr to a cspacepath, for use in
    // operations such as Untyped_Retype
    vka_cspace_make_path(&context.vka, cap, handler);

    // exec seL4_IRQControl_Get(seL4_CapIRQControl, irq, ...)
    // to get an IRQHandler cap for IRQ "irq"
    err = simple_get_IRQ_handler(&context.simple, irq, *handler);
    assert(err == 0);
}

static void set_devEp(chardev_t* dev) 
{
    // Loop through all IRQs and get the one device needs to listen to
    // We currently assume there it only needs one IRQ.
    int irq;
    for (irq = 0; irq < 256; irq++) {
        if (ps_cdev_produces_irq(&dev->dev, irq)) {
            break;
        }
    }
    printf ("irq=%d\n", irq);

    //create IRQHandler cap
    get_irqhandler_cap(irq, &dev->handler);

    /* Assign AEP to the IRQ handler. */
    /*
    UNUSED int err = seL4_IRQHandler_SetEndpoint(
            dev->handler.capPtr, dev->ep.capPtr);
    */
    UNUSED int err = seL4_IRQHandler_SetNotification(dev->handler.capPtr,context.ntfn_object.cptr);
    assert(err == 0);

    //read once: not sure why, but it does not work without it, it seems
    ps_cdev_getchar(&dev->dev);
    err = seL4_IRQHandler_Ack(dev->handler.capPtr);
    assert(err == 0);
}


static void handle_cdev_event(char *msg, chardev_t* dev) 
{
    printf("handle_cdev_event(): %s\n", msg);
    for (;;) {
        //int c = __arch_getchar();
        int c = ps_cdev_getchar(&dev->dev);
        if (c == EOF) {
            //read till we get EOF
            break;
        }
        printf("You typed [%c]\n", c);
    }

    UNUSED int err = seL4_IRQHandler_Ack(dev->handler.capPtr);
    assert(err == 0);
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


    error = !DriverKitInit(&context);
    ZF_LOGF_IFERR(error, "Failed to init DriverKit.\n");

/* Processes table & init */

    ProcessInit(&initProcess);

    initProcess._pid = 1;

    ProcessTableInit();

    ProcessTableAppend(&initProcess);



/* File Server*/

    error = !FileServerInit();
    ZF_LOGF_IFERR(error, "Failed to  init FileServer\n");

// CPIO
    error = !CPIOServerInit();
    ZF_LOGF_IFERR(error, "Failed to  init CPIO Server\n");

    error = !FileServerRegisterHandler( getCPIOServerHandler() );
    ZF_LOGF_IFERR(error, "Failed to register CPIO File System \n");

// dev

    error = !DevServerInit();
    ZF_LOGF_IFERR(error, "Failed to  init Dev Server\n");

    error = !FileServerRegisterHandler( getDevServerHandler() );
    ZF_LOGF_IFERR(error, "Failed to register Dev File System \n");

// EGA

    error = !InitEGADriver( &context);
    ZF_LOGF_IFERR(error, "Failed to  init EGA driver\n");


    error = !DevServerRegisterFile("console", EGADriverGetDeviceOps() );
    ZF_LOGF_IFERR(error, "Failed to  register 'console' EGA handler\n");


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


// Test keyboard


    sel4platsupport_get_io_port_ops(&context.opsIO.io_port_ops, &context.simple , &context.vka);

    chardev_t keyboard;

    ps_chardevice_t *ret;
    ret = ps_cdev_init(PC99_KEYBOARD_PS2, &context.opsIO, &keyboard.dev);
    assert(ret != NULL);

//    error = vka_alloc_notification(&context.vka, &keyboard.ntfn_object);
//    assert(error == 0);

//    error = seL4_TCB_BindNotification(seL4_CapInitThreadTCB, keyboard.ntfn_object.cptr);

//    seL4_Word badge1 = 1;
//    error = vka_mint_object(&context.vka, &ep_object, &keyboard.ep, seL4_AllRights, badge1);
//    assert(error == 0);


    set_devEp(&keyboard);

    printf("Init Keyboard : so far so good\n");


    for (;;) 
    {
        seL4_Word sender_badge;
        printf("waiting:\n");
        UNUSED seL4_MessageInfo_t msg = seL4_Recv(ep_object.cptr, &sender_badge);
        printf("seL4_Wait returned with badge: %d\n", sender_badge);


	handle_cdev_event("key", &keyboard);
    }

/* BEGIN PROCESS */

    Process *testProcess = ProcessAlloc();
    error = ProcessStart(&context, testProcess,"TestSysCalls", context.ep_cap_path, &initProcess, seL4_MaxPrio );
    if (error == 0)
    {
        ProcessTableAppend(testProcess);
    }
    else 
    {
	printf("Error spawning  TestSysCalls\n");
    }

    Process *process2 = ProcessAlloc();
    error = ProcessStart(&context,  process2,"shell", context.ep_cap_path, &initProcess, seL4_MaxPrio );
    if(error == 0)
    {
        ProcessTableAppend(process2);
    }

//
    printf("Init : Got %i processes \n" , ProcessTableGetCount() );

    processLoop( &context,ep_object.cptr);//ep_cap_path);


    return 0;
}
