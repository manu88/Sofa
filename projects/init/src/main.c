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

#include "Drivers/Keyboard.h"

//#define APP_PRIORITY seL4_MaxPrio
#define APP_IMAGE_NAME "app"


static InitContext context = { 0 };


static Process initProcess = {0};


static KeyboardDevice _keyboard;


void handle_cdev_event( void* _dev) 
{
    KeyboardDevice* dev = (KeyboardDevice*) _dev;

    for (;;) 
    {
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

/* DriverKit*/

   error = !DriverKitInit(&context);
   ZF_LOGF_IFERR(error, "Failed to  init DriverKit\n");

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

    error = vka_alloc_notification(&context.vka, &context.ntfn_object);
    assert(error == 0);

    error = seL4_TCB_BindNotification(seL4_CapInitThreadTCB, context.ntfn_object.cptr);
    ZF_LOGF_IFERR(error, "Unable to BindNotification.\n");




    cspacepath_t notification_path;

    vka_cspace_make_path( &context.vka, context.ntfn_object.cptr, &notification_path);

/* System Timer */

    error  = !TimerDriverInit(&context ,notification_path.capPtr);

    assert( error == 0);

    error = !TimersWheelInit(&context.timersWheel); // TimersWheelInit returns 1 on sucess -> negate
    ZF_LOGF_IFERR(error, "Unable to initialize Timers Wheel.\n");

    uint64_t timerResolution = 0;;
    error = ltimer_get_resolution(&context.timer.ltimer , &timerResolution);

    printf("Timer resolution is %lu (error %i)\n" ,timerResolution , error);

// Test keyboard

    sel4platsupport_get_io_port_ops(&context.opsIO.io_port_ops, &context.simple , &context.vka);

    error = !KeyboardDeviceInit(&context, &notification_path , &_keyboard);
    ZF_LOGF_IFERR(error, "Unable to initialize Keyboard .\n");

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

    processLoop( &context,ep_object.cptr , &_keyboard);


    return 0;
}
