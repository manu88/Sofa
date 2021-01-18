/*
 * This file is part of the Sofa project
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
#include <autoconf.h>
#define CONFIG_HAVE_TIMER 1
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#include <sel4runtime.h>

#include <allocman/vka.h>

#include <cpio/cpio.h>

#include <platsupport/local_time_manager.h>
#include <sel4runtime.h>
#include <sel4platsupport/timer.h>

#include <sel4debug/register_dump.h>
#include <sel4platsupport/device.h>
#include <sel4platsupport/platsupport.h>
#include <platsupport/chardev.h>

#include <sel4utils/stack.h>


#include <simple/simple.h>

#include <utils/util.h>

#include <vka/object.h>
#include <vka/capops.h>

#include <vspace/vspace.h>
#include "Environ.h"
#include "Process.h"


#include "Syscalls/SyscallTable.h"
#include "Allocator.h"
#include "Timer.h"


#include "DeviceKit/DeviceTree.h"
#include "NameServer.h"
#include <sel4platsupport/arch/io.h>
#include "KThread.h"
#include <Sofa.h>
#include <signal.h>
#include "VFS.h"
#include "devFS.h"
#include "fakefs.h"
#include "cpio.h"

#include "DeviceKit.h"
#include "DKService.h"
#include "VFSService.h"
#include "NetService.h"
#include "ProcService.h"

/* Stub KThread instance for the main kernel_task thread, that CANNOT sleep.
Calls to KSleep will ensure that they are never called from main*/
extern KThread _mainThread;


static char kernelTaskName[] = "kernel_task";

Process initProcess;

static void process_messages()
{
    KernelTaskContext* env = getKernelTaskContext();
    while (1)
    {   
        seL4_Word badge = 0;
        seL4_MessageInfo_t info = seL4_Recv(env->root_task_endpoint.cptr, &badge);
        seL4_Word label = seL4_MessageInfo_get_label(info);
        if(label == seL4_NoFault)
        {
            if(badge == TIMER_BADGE)
            {
                seL4_IRQHandler_Ack(env->timer_irqs[0].handler_path.capPtr);
                tm_update(&env->tm);
            }
            else 
            {
                const ThreadBase* base = (ThreadBase*) badge;
                Thread* caller = (Thread*) badge;
                SyscallID rpcID = seL4_GetMR(0);  
                if(rpcID > 0 && rpcID < SyscallID_Last)
                {
                    Syscall_perform(rpcID, caller, info);
                }
                else
                {
                    assert(0);
                }
                
            }
        }
        else if (label == seL4_CapFault)
        {
            Thread* sender = (Thread*) badge;
            Process* process = sender->_base.process;
            KLOG_ERROR("Got cap fault from '%s' %i\n", ProcessGetName(process), process->init->pid);
        }
        else if (label == seL4_VMFault)
        {
            const ThreadBase* base = (ThreadBase*) badge;
            if(base->kernTaskThread)
            {
                KLOG_ERROR("Fault in kernel_task thread\n");
                assert(0);
                continue;
            }
            Thread* sender = (Thread*) badge;
            Process* process = sender->_base.process;
            KLOG_INFO("Got VM fault from %i %s in thread %p\n",
                   ProcessGetPID(process),
                   ProcessGetName(process),
                   sender == &process->main? 0: sender
                   );

            const seL4_Word programCounter      = seL4_GetMR(seL4_VMFault_IP);
            const seL4_Word faultAddr           = seL4_GetMR(seL4_VMFault_Addr);
            const seL4_Word isPrefetch          = seL4_GetMR(seL4_VMFault_PrefetchFault);
            const seL4_Word faultStatusRegister = seL4_GetMR(seL4_VMFault_FSR);
            
            KLOG_INFO("[kernel_task] programCounter      0X%lX\n",programCounter);
            KLOG_INFO("[kernel_task] faultAddr           0X%lX\n",faultAddr);
            KLOG_INFO("[kernel_task] isPrefetch          0X%lX\n",isPrefetch);
            KLOG_INFO("[kernel_task] faultStatusRegister 0X%lX\n",faultStatusRegister);

            doExit(process, MAKE_EXIT_CODE(0, SIGSEGV));
        }
        else 
        {
            KLOG_TRACE("Received message with label %lu from %lu\n", label, badge);
        }   
    }
}

void *main_continued(void *arg UNUSED)
{
    int error;

    KLOG_INFO("\n------Sofa------\n");
    KLOG_INFO("----------------\n");

    seL4_SetUserData((seL4_Word) &_mainThread);
    KernelTaskContext* env = getKernelTaskContext();

    error = vka_alloc_endpoint(&env->vka, &env->root_task_endpoint);
    assert(error == 0);

    if (config_set(CONFIG_KERNEL_MCS)) 
    {
        error = vka_alloc_reply(&env->vka, &env->reply);
        ZF_LOGF_IF(error, "Failed to allocate reply");
    }

    env->_sysState = SystemState_Running;
// Base system init    
    error = NameServerInit();
    assert(error == 0);

    error = DeviceKitInit();
    assert(error == 0);


// base services init

    error = ProcServiceInit();
    assert(error == 0);

    error = ProcServiceStart();
    assert(error == 0);

    error = VFSInit();
    assert(error == 0);

    KLOG_INFO("Starting NetService\n");
    error = NetServiceInit();
    assert(error == 0);

// device related things
    error = DeviceTreeInit();
    assert(error == 0);

    KLOG_INFO("Starting DeviceKit Service\n");
    error = DKServiceInit();
    assert(error == 0);

#if 0
    error = SerialInit();
    assert(error == 0);

    error = SerialInit2();
    assert(error == 0);
#endif

    KLOG_INFO("Starting VFSService\n");
    error = VFSServiceInit();
    assert(error == 0);

    error = VFSServiceStart();
    assert(error == 0);

    error = NetServiceStart();
    assert(error == 0);

    error = DKServiceStart();
    assert(error == 0);

// 'user-space' bootstrap
    VFSMount(getFakeFS(), "/fake", &error);
    VFSMount(getCpioFS(), "/cpio", &error);
    VFSMount(getDevFS(), "/dev", &error);    




    ProcessInit(&initProcess);
    initProcess.argc = 0;
    spawnApp(&initProcess, "/cpio/init", NULL);

    seL4_DebugDumpScheduler();
    process_messages();    

    return NULL;
}

/* Note that the following globals are place here because it is not expected that
 * this function be refactored out of sel4test-driver in its current form. */
/* Number of objects to track allocation of. Currently all serial devices are
 * initialised with a single Frame object.  Future devices may need more than 1.
 */
#define NUM_ALLOC_AT_TO_TRACK 1
/* Static global to store the original vka_utspace_alloc_at function. It
 * isn't expected for this to dynamically change after initialisation.*/
static vka_utspace_alloc_at_fn vka_utspace_alloc_at_base;
/* State that serial_utspace_alloc_at_fn uses to determine whether to cache
 * allocations. It is intended that this flag gets set before the serial device
 * is initialised and then unset afterwards. */
static bool serial_utspace_record = false;

typedef struct uspace_alloc_at_args {
    uintptr_t paddr;
    seL4_Word type;
    seL4_Word size_bits;
    cspacepath_t dest;
} uspace_alloc_at_args_t;
/* This instance of vka_utspace_alloc_at_fn will keep a record of allocations up
 * to NUM_ALLOC_AT_TO_TRACK while serial_utspace_record is set. When serial_utspace_record
 * is unset, any allocations matching recorded allocations will instead copy the cap
 * that was originally allocated. These subsequent allocations cannot be freed using
 * vka_utspace_free and instead the caps would have to be manually deleted.
 * Freeing these objects via vka_utspace_free would require also wrapping that function.*/
static int serial_utspace_alloc_at_fn(void *data, const cspacepath_t *dest, seL4_Word type, seL4_Word size_bits,
                                      uintptr_t paddr, seL4_Word *cookie)
{
    static uspace_alloc_at_args_t args_prev[NUM_ALLOC_AT_TO_TRACK] = {};
    static size_t num_alloc = 0;

    ZF_LOGF_IF(!vka_utspace_alloc_at_base, "vka_utspace_alloc_at_base not initialised.");
    if (!serial_utspace_record) {
        for (int i = 0; i < num_alloc; i++) {
            if (paddr == args_prev[i].paddr &&
                type == args_prev[i].type &&
                size_bits == args_prev[i].size_bits) {
                return vka_cnode_copy(dest, &args_prev[i].dest, seL4_AllRights);
            }
        }
        return vka_utspace_alloc_at_base(data, dest, type, size_bits, paddr, cookie);
    } else {
        ZF_LOGF_IF(num_alloc >= NUM_ALLOC_AT_TO_TRACK, "Trying to allocate too many utspace objects");
        int ret = vka_utspace_alloc_at_base(data, dest, type, size_bits, paddr, cookie);
        if (ret) {
            return ret;
        }
        uspace_alloc_at_args_t a = {.paddr = paddr, .type = type, .size_bits = size_bits, .dest = *dest};
        args_prev[num_alloc] = a;
        num_alloc++;
        return ret;
    }
}

int main(void)
{
    _mainThread._base.process = getKernelTaskProcess();
    seL4_SetUserData((seL4_Word) &_mainThread);
    KernelTaskContext* env = getKernelTaskContext();

    getKernelTaskProcess()->name = kernelTaskName;
    int error;

    error = sel4platsupport_new_io_ops(&env->vspace, &env->vka, &env->simple, &env->ops);
    ZF_LOGF_IF(error, "Failed to initialise IO ops");
    assert(error == 0);

#ifdef CONFIG_DEBUG_BUILD
    seL4_DebugNameThread(seL4_CapInitThreadTCB, "kernel_task");
#endif

    error = IOInit();
    assert(error == 0);


    error = TimerInit();
    assert(error == 0);

    void *res;
    error = sel4utils_run_on_stack(&env->vspace, main_continued, NULL, &res);
    assert(error == 0);
    assert(res == 0);

    return 0;
}

