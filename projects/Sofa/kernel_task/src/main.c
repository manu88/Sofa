/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

/* Include Kconfig variables. */
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
#include <sel4utils/process.h>


#include <simple/simple.h>

#include <utils/util.h>

#include <vka/object.h>
#include <vka/capops.h>

#include <vspace/vspace.h>
#include "Environ.h"
#include "testtypes.h"

#include <Sofa.h>
#include "Syscalls/SyscallTable.h"
#include "Allocator.h"
#include "Timer.h"
#include "Serial.h"
#include "DeviceTree.h"
#include <sel4platsupport/arch/io.h>




extern char _cpio_archive[];
extern char _cpio_archive_end[];


Process initProcess;

static void DumpProcesses()
{
    Process* p = NULL;
    printf("----- List process -----\n");
    FOR_EACH_PROCESS(p)
    {
        printf("%i '%s' %i threads\n", ProcessGetPID(p), ProcessGetName(p), ProcessCountExtraThreads(p));
    }
    printf("------------------------\n");

    seL4_DebugDumpScheduler();
}


static void process_messages()
{
    KernelTaskContext* env = getKernelTaskContext();
    while (1)
    {   
        handleSerialInput(env);

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
#if 0
            else if(badge == SERIAL_BADGE)
            {
                seL4_IRQHandler_Ack(env->handler.capPtr);
                printf("IRQ\n");
                //ps_cdev_handle_irq(&env->comDev, -1);
/*                int data = 0;
                while(data != EOF)
                {  
                    data = ps_cdev_getchar(&env->comDev);
                    if(data > 0)
                        printf("%c\n", data);
                }
*/                
            }
#endif            
            else 
            {
                Thread* caller = (Thread*) badge;
                Process* process = caller->process;
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
            Process* process = sender->process;
            printf("Got cap fault from '%s' %i\n", process->init->name, process->init->pid);
        }
        else if (label == seL4_VMFault)
        {
            Thread* sender = (Thread*) badge;
            Process* process = sender->process;
            printf("Got VM fault from %i %s in thread %p\n",
                   ProcessGetPID(process),
                   ProcessGetName(process),
                   sender == &process->main? 0: sender
                   );

            const seL4_Word programCounter      = seL4_GetMR(seL4_VMFault_IP);
            const seL4_Word faultAddr           = seL4_GetMR(seL4_VMFault_Addr);
            const seL4_Word isPrefetch          = seL4_GetMR(seL4_VMFault_PrefetchFault);
            const seL4_Word faultStatusRegister = seL4_GetMR(seL4_VMFault_FSR);
            
            printf("[kernel_task] programCounter      0X%lX\n",programCounter);
            printf("[kernel_task] faultAddr           0X%lX\n",faultAddr);
            printf("[kernel_task] isPrefetch          0X%lX\n",isPrefetch);
            printf("[kernel_task] faultStatusRegister 0X%lX\n",faultStatusRegister);

            doExit(process, -1);


        }
        else 
        {
            printf("Received message with label %lu from %lu\n", label, badge);
        }   
    }
}




void *main_continued(void *arg UNUSED)
{

    /* Print welcome banner. */
    printf("\n------Sofa------\n");
    printf("----------------\n");
    int error;

    KernelTaskContext* env = getKernelTaskContext();

    error = vka_alloc_endpoint(&env->vka, &env->root_task_endpoint);
    assert(error == 0);

#if 0
    /* allocate a piece of device untyped memory for the frame tests,
     * note that spike doesn't have any device untypes so the tests that require device untypes are turned off */
    if (!config_set(CONFIG_PLAT_SPIKE)) {
        assert(0);
        bool allocated = false;
        int untyped_count = simple_get_untyped_count(&env.simple);
        for (int i = 0; i < untyped_count; i++) {
            bool device = false;
            uintptr_t ut_paddr = 0;
            size_t ut_size_bits = 0;
            seL4_CPtr ut_cptr = simple_get_nth_untyped(&env.simple, i, &ut_size_bits, &ut_paddr, &device);
            if (device) {
                error = vka_alloc_frame_at(&env.vka, seL4_PageBits, ut_paddr, &env.device_obj);
                if (!error) {
                    allocated = true;
                    /* we've allocated a single device frame and that's all we need */
                    break;
                }
            }
        }
        ZF_LOGF_IF(allocated == false, "Failed to allocate a device frame for the frame tests");
    }
#endif
    /* allocate lots of untyped memory for tests to use */

    /* Allocate a reply object for the RT kernel. */
    if (config_set(CONFIG_KERNEL_MCS)) {
        error = vka_alloc_reply(&env->vka, &env->reply);
        ZF_LOGF_IF(error, "Failed to allocate reply");
    }

    sel4platsupport_get_io_port_ops(&env->ops.io_port_ops, &env->simple, &env->vka);

    error = DeviceTreeInit();
    assert(error == 0);

    error = SerialInit();
    assert(error == 0);

    ProcessInit(&initProcess);
    spawnApp(&initProcess, "shell", NULL);
/*
    for(int i=0;i<40;i++)
    {
        Process* p = malloc(sizeof(Process));
        assert(p);
        ProcessInit(p);
        spawnApp(p, "app", &initProcess);
    }
    seL4_DebugDumpScheduler();
*/
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
    //assert(0);
    InitEnv();

    KernelTaskContext* env = getKernelTaskContext();
    int error;


#ifdef CONFIG_DEBUG_BUILD
    seL4_DebugNameThread(seL4_CapInitThreadTCB, "kernel_task");
#endif

    error = TimerInit();
    assert(error == 0);

    simple_print(&env->simple);

    /* switch to a bigger, safer stack with a guard page
     * before starting the tests */
    printf("Switching to a safer, bigger stack... ");
    fflush(stdout);
    void *res;

    /* Run sel4test-test related tests */
    error = sel4utils_run_on_stack(&env->vspace, main_continued, NULL, &res);
    assert(error == 0);
    assert(res == 0);

    return 0;
}

