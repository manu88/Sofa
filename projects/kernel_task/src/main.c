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
//#include <sel4platsupport/arch/io.h>

#include "Bootstrap.h"
#include "ProcessDef.h"
#include "ProcessTable.h"
#include "DriverKit.h"
#include "Utils.h"
#include "MainLoop.h"
#include "Credentials.h"
#include "FileServer.h"
#include "CpioServer.h"
#include "DevServer.h"

#include "Drivers/EGADriver.h"
#include "Drivers/Keyboard.h"
#include "Drivers/Fat.h"
#include "Drivers/Usb.h"
#include "Devices/Null.h"
#include "Devices/Terminal.h"
#include "SysHandler.h"
#include "Timer.h"
#include "Stats.h"


#include "Thread.h"

static char taskName[] =  "kernel_task";

static KernelTaskContext context = { 0 };


static Process kernTaskProcess = {0};

static Terminal _terminal;

static vka_object_t ep_object = {0};

static     Thread thread;


static int addDefaultDevices(void)
{
	int error =  !DevNullInit();
	assert(error == 0);
	return 0;
}


static void ThreadTest(Thread *self, void *arg, void *ipc_buf)
{
	printf("Thread test Started\n");
	assert(self == &thread);

	while(1)
 	{
		
	}
}

static int lateSystemInit()
{
    int error = 0;

/* start thread test*/

    sel4utils_thread_config_t threadConf = thread_config_new(&context.simple);

    if(ThreadInit(&thread , &context.vka , &context.vspace , threadConf))
    {
	error = ThreadSetPriority(&thread  , seL4_MaxPrio);
	assert(error == 0);

	thread.entryPoint = ThreadTest;
	error = ThreadStart(&thread , NULL , 1);
	assert(error == 0);
    }

    else 
    {
	printf("Unable to create thread \n");
	assert( 0);
    }

/* END start thread test*/


    /* BEGIN INIT PROCESS */
    printf("Kernel_task : start init process\n");

    Process initProcess;

    ProcessInit( &initProcess );
    initProcess.currentDir =  FileServerGetRootNode();

    error = ProcessTableAddAndStart(&context, &initProcess,"init", context.ep_cap_path, &kernTaskProcess, seL4_MaxPrio );// !ProcessTableAppend(process2);
    assert(error == 0);


    TestStats();


    seL4_DebugDumpScheduler();
    processLoop( &context,ep_object.cptr );
}


int main(void)
{

    memset(&context , 0 , sizeof(KernelTaskContext) );

    zf_log_set_tag_prefix("kernel_task");

    UNUSED int error = 0;

    context.info = platsupport_get_bootinfo();
    ZF_LOGF_IF(context.info == NULL, "Failed to get bootinfo.");

    error = bootstrapSystem( &context);
    ZF_LOGF_IFERR(error, "Failed to bootstrap system.\n");

    assert(error == 0);

/* create an endpoint. */
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

    error = TimerInit(&context , notification_path.capPtr);
    assert( error == 0);




/* File Server*/

    error = !FileServerInit();
    ZF_LOGF_IFERR(error, "Failed to  init FileServer\n");


/* DriverKit*/

    error = !DriverKitInit(&context);
    ZF_LOGF_IFERR(error, "Failed to init DriverKit.\n");

/* Processes table & init */

    ProcessInit(&kernTaskProcess);

    kernTaskProcess._pid = 0;
    kernTaskProcess.cmdLine = taskName;

    ProcessTableInit();

    ProcessTableAppend(&kernTaskProcess);
    assert(kernTaskProcess._pid == 0); // kernel_task MUST BE the first task

    error = !FileServerAddNodeAtPath(ProcessTableGetInode() , "/");
    ZF_LOGF_IFERR(error, "unable to add 'proc' fs to filesystem \n");


// CPIO

    error = !CPIOServerInit();
    ZF_LOGF_IFERR(error, "Failed to  init CPIO Server\n");



    error = !FileServerAddNodeAtPath( CPIOServerGetINode() , "/");//   FileServerRegisterHandler( getCPIOServerHandler() , "cpio" );
    ZF_LOGF_IFERR(error, "Failed to register CPIO File System \n");

    assert(InodeGetChildrenCount(FileServerGetRootNode()) > 0);
// dev

    error = !DevServerInit();
    ZF_LOGF_IFERR(error, "Failed to  init Dev Server\n");

//    error = !FileServerRegisterHandler( getDevServerHandler() , "dev" );
    error = !FileServerAddNodeAtPath(DevServerGetInode(), "/");
    ZF_LOGF_IFERR(error, "Failed to register Dev File System \n");

    assert( FileServerGetINodeForPath("/dev/" , NULL) == DevServerGetInode() );

    assert(InodeGetChildrenCount(FileServerGetRootNode()) > 0);


    error = !SysHandlerInit();
    assert(error == 0);

    error = !FileServerAddNodeAtPath( SysHandlerGetINode() , "/");
    ZF_LOGF_IFERR(error, "Failed to register Sys File System \n");


    error = addDefaultDevices();
    assert( error == 0);

// USB
//    error = USBInit( &context);
//    assert(error == 0);

// Fat driver
    FatTestInit();
// Default terminal (EGA + keyboard)

    error = !TerminalInit( &context, &notification_path, &_terminal);
    ZF_LOGF_IFERR(error, "Unable to initialize  terminal\n");
    assert( error == 0);

    error = !DriverKitRegisterDevice(IRQ_BADGE_KEYBOARD, (IOBaseDevice*) &_terminal);
    ZF_LOGF_IFERR(error, "Unable to initialize  terminal driver \n");
    assert( error == 0);

    assert( DriverKitGetDeviceForBadge(IRQ_BADGE_KEYBOARD) == (IOBaseDevice*) &_terminal);

    error = !DevServerRegisterFile( (Inode*)&_terminal.node );// !DevServerRegisterFile("console", &_terminal.devOps );// EGADriverGetDeviceOps() );
    ZF_LOGF_IFERR(error, "Failed to  register 'console' EGA handler\n");

    assert(FileServerGetINodeForPath("/dev/console" , NULL) == &_terminal.node);

    error = lateSystemInit();
/*
    {
    printf("Kernel_task : start init process\n");

    Process initProcess;

    ProcessInit( &initProcess );
    initProcess.currentDir =  FileServerGetRootNode();

    error = ProcessTableAddAndStart(&context, &initProcess,"init", context.ep_cap_path, &kernTaskProcess, seL4_MaxPrio );// !ProcessTableAppend(process2);
    assert(error == 0);




    TestStats();

    printf("Kernel_task : start runloop \n");


    processLoop( &context,ep_object.cptr );
    }
*/
    return 0;
}
