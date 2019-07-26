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
#include <sel4utils/process.h>
#include <cpio/cpio.h>
#include <assert.h>


#include "Utils.h"
#include "Process.h"
#include "Bootstrap.h"
#include "Timer.h"
#include "ProcessSysCall.h"
#include "NameServer.h"
#include "Config.h"
extern char _cpio_archive[];
extern char _cpio_archive_end[];


//static vka_object_t rootTaskEP = {0};
static vka_object_t ntfn_object;
//static ps_chardevice_t comDev;

static Process initProcess = {0};

static int initRootEndPoint()
{
	int error = vka_alloc_endpoint( getVka(), &getKernelTaskContext()->rootTaskEP);
	if( error != 0)
	{
		printf("vka_alloc_endpoint for RootEndPoint failed %i\n" , error);

		return error;
	}
    
	cspacepath_t ep_cap_path;
	vka_cspace_make_path( getVka(), getKernelTaskContext()->rootTaskEP.cptr, &ep_cap_path);

	return error;
}

static void earlyInit()
{
	int error = bootstapSystem(); // This MUST be the first thing done!

    printf("[kernel_task] bootstrap returned %i\n" , error);

    int ret = seL4_TCB_SetPriority(simple_get_tcb(getSimple()) , simple_get_tcb(getSimple()) , SOFA_DEFAULT_PRIORITY);
    
	error = initRootEndPoint();
	printf("[kernel_task] root EndPoint returned %i\n" , error);
    
    
/* io ops */
    ps_io_ops_t ops;
    
/* timer notifications */
    error = vka_alloc_notification( getVka(), &ntfn_object);
    assert(error == 0);
    
    error = seL4_TCB_BindNotification(seL4_CapInitThreadTCB, ntfn_object.cptr);
    if (error != 0)
    {
        printf("Unable to BindNotification.\n");
        assert(0);
    }
    assert(error == 0);
    
    cspacepath_t notification_path;
    vka_cspace_make_path( getVka(), ntfn_object.cptr, &notification_path);
    
    
    error = TimerInit(&ops,notification_path.capPtr);
    assert(error == 0);
    
    // we reserve the timer ID 0 for us
    // so that threads always have a timerID > 0 so that we can distinguish unallocated ids(0)
    assert(tm_alloc_id_at( getTM() , 0) == 0);
    
    
    ps_cdev_init(PC99_SERIAL_COM1 , getIO_OPS() ,&getKernelTaskContext()->comDev);
}



static void lsCPIO()
{
	struct cpio_info info;
	unsigned long len = _cpio_archive_end - _cpio_archive;

	if(cpio_info(_cpio_archive, len , &info) != 0)
    	{
	        printf("Error gettings CPIO archive\n");
		return;
	}

	printf("----- CPIO Content (%i files) --- \n" , info.file_count);
    
    return ;
	char **buf = malloc( info.file_count);
	
	for(int i=0;i<info.file_count;i++)
    	{
        	buf[i] = malloc(info.max_path_sz );
        	memset(buf[i] , 0 , info.max_path_sz);
	}

	cpio_ls(_cpio_archive ,len, buf, info.file_count);
    
    
	for(int i=0;i<info.file_count;i++)
    	{
        	printf("'%s'\n" , buf[i]);
        	free(buf[i]);
    	}
    
	free(buf);

	printf("----- END CPIO Content --- \n");
}

static int OnTime(uintptr_t token)
{
    //printf("On time\n");
    //seL4_DebugDumpScheduler();
}

static void lateInit()
{
    printf("##########################################\n");
    printf("#                 Sofa OS                #\n");
    printf("##########################################\n");
    lsCPIO();
    
    int ret = ProcessListInit();
    assert(ret == 0);
    printf("[kernel_task] init name server\n");
    ret = NameServerInitSystem();
    assert(ret == 0);
    
    
    
    assert(getKernelTaskContext()->rootTaskEP.cptr != 0);

    ProcessInit(&initProcess);
    
    if(ProcessStart(&initProcess , "init" ,&getKernelTaskContext()->rootTaskEP , NULL) != 0)
    {
        printf("[kernel_task] error starting init process\n");
    }
    assert(initProcess.pid == 1); // MUST BE 1
 
    
    //ret = tm_register_periodic_cb( getTM() , 10000000000 ,0, 0 , OnTime ,(uintptr_t) 0);
    //assert(ret == 0);
}





#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER    (1 << 0)



static void run()
{
	printf("[kernel_task] start runloop\n");
	while(1)
	{

//		seL4_DebugDumpScheduler();
		seL4_Word sender_badge = 0;
        seL4_MessageInfo_t message = seL4_Recv(getKernelTaskContext()->rootTaskEP.cptr, &sender_badge);

        seL4_Word label = seL4_MessageInfo_get_label(message);

        if(sender_badge & IRQ_EP_BADGE)
        {
            
            if (sender_badge & IRQ_BADGE_TIMER)
            {
            }
            
            TimerProcess(sender_badge);
            
        }
        else
        {
            Process* sender = ProcessGetByPID(sender_badge);
            
            if( label == seL4_NoFault)
            {
                assert(sender);
                processSysCall(sender,message , sender_badge);
            }
            else if( label == seL4_CapFault)
            {
                printf("[kernel_task] seL4_CapFault from %i %s\n" , sender->pid,  ProcessGetName(sender));
            }
            else if (label == seL4_VMFault)
            {
                printf("[kernel_task] seL4_VMFault from %i %s\n" , sender->pid,  ProcessGetName(sender));
                ProcessKill(sender , SofaSignal_VMFault);
            }
            else if(label == seL4_UnknownSyscall)
            {
                printf("[kernel_task] seL4_UnknownSyscall from %i %s\n" , sender->pid,  ProcessGetName(sender));
            }
            else if(label == seL4_UserException)
            {
                printf("[kernel_task] seL4_UserException from %i %s\n" , sender->pid,  ProcessGetName(sender));
            }
            else
            {
                printf("[kernel_task] Other msg label %lx\n" , label);
            }
        }
        
	}


}


int main()
{
	printf("[kernel_task] started\n");

	earlyInit();
	lateInit();
	run();
	// never returns
	return 0;
}
