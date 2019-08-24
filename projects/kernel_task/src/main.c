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

#include "Config.h"
extern char _cpio_archive[];
extern char _cpio_archive_end[];


//static vka_object_t rootTaskEP = {0};
static vka_object_t ntfn_object;
//static ps_chardevice_t comDev;
static ps_io_ops_t ops = {{0}};

static Process initProcess = {0};

static int initRootEndPoint()
{
	int error = vka_alloc_endpoint( getVka(), &getKernelTaskContext()->rootTaskEP);
	if( error != 0)
	{
		klog("vka_alloc_endpoint for RootEndPoint failed %i\n" , error);

		return error;
	}
    
	//cspacepath_t ep_cap_path;
	//vka_cspace_make_path( getVka(), getKernelTaskContext()->rootTaskEP.cptr, &ep_cap_path);

	return error;
}

static void earlyInit()
{
	int error = bootstrapSystem(); // This MUST be the first thing done!
    assert(error == 0);
    
    int ret = seL4_TCB_SetPriority(simple_get_tcb(getSimple()) , simple_get_tcb(getSimple()) , SOFA_DEFAULT_PRIORITY);
    
    klog("[kernel_task] init platform I/O ops\n");
    assert(bootstrapIO() == 0);
    
	error = initRootEndPoint();
    assert(error == 0);

/* timer notifications */
    error = vka_alloc_notification( getVka(), &ntfn_object);
    assert(error == 0);
    
    error = seL4_TCB_BindNotification(seL4_CapInitThreadTCB, ntfn_object.cptr);
    if (error != 0)
    {
        klog("Unable to BindNotification.\n");
        assert(0);
    }
    assert(error == 0);
    
    cspacepath_t notification_path;
    vka_cspace_make_path( getVka(), ntfn_object.cptr, &notification_path);

/* io ops */
/*
    klog("[kernel_task] init Timer\n");
    error = TimerInit(&ops,notification_path.capPtr);
    assert(error == 0);
    klog("[kernel_task] init Timer OK \n");
    // we reserve the timer ID 0 for us
    // so that threads always have a timerID > 0 and we can distinguish unallocated ids(0)
    assert(TimerAllocIDAt( 0)  == 0);
 
 */
    klog("[kernel_task] init COM1\n");
    ps_cdev_init(PC99_SERIAL_COM1 , getIO_OPS() ,&getKernelTaskContext()->comDev);
}

static void lateInit()
{
    klog("##########################################\n");
    klog("#                 Sofa OS                #\n");
    klog("##########################################\n");
    klog("Version %.2i.%.2i.%.2i\n" , SOFA_VERSION_MAJ , SOFA_VERSION_MIN , SOFA_VERSION_PATCH);

    
    int ret = ProcessListInit();
    assert(ret == 0);

    assert(getKernelTaskContext()->rootTaskEP.cptr != 0);

    ProcessInit(&initProcess);
    // give init a few caps
    initProcess.caps.caps = SofaCap_Spawn;
    
    if(ProcessStart(&initProcess , "init" ,&getKernelTaskContext()->rootTaskEP , NULL) != 0)
    {
        klog("[kernel_task] error starting init process\n");
        assert(0);
    }
    assert(initProcess.pid == 1); // MUST BE 1
 
}





#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER    (1 << 0)



static void run()
{
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
                klog("[kernel_task] seL4_CapFault from %i %s\n" , sender->pid,  ProcessGetName(sender));
                
                const seL4_Word addr            = seL4_GetMR(seL4_CapFault_IP); // Addr to restart
                const seL4_Word capAddr         = seL4_GetMR(seL4_CapFault_Addr); // Capability address
                const seL4_Word isInReceivPhase = seL4_GetMR(seL4_CapFault_InRecvPhase); // In receive phase (1 if the fault happened during a receive system call, 0 otherwise)
                const seL4_Word lookupFailureDesc = seL4_GetMR(seL4_CapFault_LookupFailureType);
                
                klog("[kernel_task] addr               %lu\n",addr);
                klog("[kernel_task] capAddr            %lu\n",capAddr);
                klog("[kernel_task] isInReceivPhase    %lu\n",isInReceivPhase);
                klog("[kernel_task] lookupFailureDesc  %lu\n",lookupFailureDesc);
                
            }
            else if (label == seL4_VMFault)
            {
                klog("[kernel_task] seL4_VMFault from pid%i '%s'\n" ,
                       sender->pid,
                       ProcessGetName(sender)
                       );
                
                const seL4_Word programCounter      = seL4_GetMR(seL4_VMFault_IP);
                const seL4_Word faultAddr           = seL4_GetMR(seL4_VMFault_Addr);
                const seL4_Word isPrefetch          = seL4_GetMR(seL4_VMFault_PrefetchFault);
                const seL4_Word faultStatusRegister = seL4_GetMR(seL4_VMFault_FSR);
                
                klog("[kernel_task] programCounter      %lu\n",programCounter);
                klog("[kernel_task] faultAddr           %lu\n",faultAddr);
                klog("[kernel_task] isPrefetch          %lu\n",isPrefetch);
                klog("[kernel_task] faultStatusRegister %lu\n",faultStatusRegister);
                
                // TODO : handle vm error
                
                // kernel_task or init : we're screwed
                if(sender->pid < 2 )
                {
                    Panic("init returned");
                    // no return
                    assert(0);
                }
                
                ProcessKill(sender , SofaSignal_VMFault);
            }
            else if(label == seL4_UnknownSyscall)
            {
                klog("[kernel_task] seL4_UnknownSyscall from %i %s\n" , sender->pid,  ProcessGetName(sender));
            }
            else if(label == seL4_UserException)
            {
                processUserException(sender, message , sender_badge);
            }
            else
            {
                klog("[kernel_task] Other msg label %lx\n" , label);
            }
        }
        
	}


}

int main()
{
	klog("[kernel_task] started\n");

	earlyInit();
	lateInit();
	run();
	// never returns
	return 0;
}
