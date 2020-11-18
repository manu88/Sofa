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

#include <allocman/bootstrap.h>
#include <allocman/vka.h>

#include <cpio/cpio.h>

#include <platsupport/local_time_manager.h>
#include <sel4runtime.h>
#include <sel4platsupport/timer.h>

#include <sel4debug/register_dump.h>
#include <sel4platsupport/device.h>
#include <sel4platsupport/platsupport.h>
#include <platsupport/chardev.h>
#include <sel4utils/vspace.h>
#include <sel4utils/stack.h>
#include <sel4utils/process.h>


#include <simple/simple.h>
#include <simple-default/simple-default.h>

#include <utils/util.h>

#include <vka/object.h>
#include <vka/capops.h>

#include <vspace/vspace.h>
#include "Environ.h"
#include "testtypes.h"

#include <sel4platsupport/io.h>
#include <Sofa.h>
#include "Syscalls/SyscallTable.h"
#include "Allocator.h"
#include "Timer.h"
#include "Serial.h"
#include "DeviceTree.h"
#include <sel4platsupport/arch/io.h>





/* ammount of untyped memory to reserve for the driver (32mb) */
#define DRIVER_UNTYPED_MEMORY (1 << 25)
/* Number of untypeds to try and use to allocate the driver memory.
 * if we cannot get 32mb with 16 untypeds then something is probably wrong */
#define DRIVER_NUM_UNTYPEDS 16

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 100)

/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 20)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* static memory for virtual memory bootstrapping */
static sel4utils_alloc_data_t data;


extern char _cpio_archive[];
extern char _cpio_archive_end[];


Process initProcess;



/* initialise our runtime environment */
static void init_env()
{
    KernelTaskContext *env = getKernelTaskContext();
    allocman_t *allocman;
    reservation_t virtual_reservation;
    int error;

    /* create an allocator */
    allocman = bootstrap_use_current_simple(&env->simple, ALLOCATOR_STATIC_POOL_SIZE, allocator_mem_pool);
    if (allocman == NULL) {
        ZF_LOGF("Failed to create allocman");
    }

    /* create a vka (interface for interacting with the underlying allocator) */
    allocman_make_vka(&env->vka, allocman);

    /* create a vspace (virtual memory management interface). We pass
     * boot info not because it will use capabilities from it, but so
     * it knows the address and will add it as a reserved region */
    error = sel4utils_bootstrap_vspace_with_bootinfo_leaky(&env->vspace,
                                                           &data, simple_get_pd(&env->simple),
                                                           &env->vka, platsupport_get_bootinfo());
    if (error) {
        ZF_LOGF("Failed to bootstrap vspace");
    }

    /* fill the allocator with virtual memory */
    void *vaddr;
    virtual_reservation = vspace_reserve_range(&env->vspace,
                                               ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights, 1, &vaddr);
    if (virtual_reservation.res == 0) {
        ZF_LOGF("Failed to provide virtual memory for allocator");
    }

    bootstrap_configure_virtual_pool(allocman, vaddr,
                                     ALLOCATOR_VIRTUAL_POOL_SIZE, simple_get_pd(&env->simple));

    error = sel4platsupport_new_io_ops(&env->vspace, &env->vka, &env->simple, &env->ops);
    ZF_LOGF_IF(error, "Failed to initialise IO ops");
}

/* Free a list of objects */
static void free_objects(vka_object_t *objects, unsigned int num)
{
    for (unsigned int i = 0; i < num; i++) {
        vka_free_object(&getKernelTaskContext()->vka, &objects[i]);
    }
}

/* Allocate untypeds till either a certain number of bytes is allocated
 * or a certain number of untyped objects */
static unsigned int allocate_untypeds(vka_object_t *untypeds, size_t bytes, unsigned int max_untypeds)
{
    unsigned int num_untypeds = 0;
    size_t allocated = 0;

    /* try to allocate as many of each possible untyped size as possible */
    for (uint8_t size_bits = seL4_WordBits - 1; size_bits > PAGE_BITS_4K; size_bits--) {
        /* keep allocating until we run out, or if allocating would
         * cause us to allocate too much memory*/
        while (num_untypeds < max_untypeds &&
               allocated + BIT(size_bits) <= bytes &&
               vka_alloc_untyped(&getKernelTaskContext()->vka, size_bits, &untypeds[num_untypeds]) == 0) {
            allocated += BIT(size_bits);
            num_untypeds++;
        }
    }
    return num_untypeds;
}

/* extract a large number of untypeds from the allocator */
static unsigned int populate_untypeds(vka_object_t *untypeds)
{
    /* First reserve some memory for the driver */
    vka_object_t reserve[DRIVER_NUM_UNTYPEDS];
    unsigned int reserve_num = allocate_untypeds(reserve, DRIVER_UNTYPED_MEMORY, DRIVER_NUM_UNTYPEDS);
    printf("Allocated %i untypeds for kernel_task\n", reserve_num);
    /* Now allocate everything else for the tests */

    unsigned int num_untypeds = allocate_untypeds(getUntypeds(), UINT_MAX, CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS);
    /* Fill out the size_bits list */
    for (unsigned int i = 0; i < num_untypeds; i++) {
        GetUntypedSizeBitsList()[i] = untypeds[i].size_bits;
    }

    /* Return reserve memory */
    free_objects(reserve, reserve_num);

    /* Return number of untypeds for tests */
    if (num_untypeds == 0) {
        ZF_LOGF("No untypeds for tests!");
    }

    return num_untypeds;
}



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
                    syscallTable[rpcID](caller, info);
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
    env->num_untypeds = populate_untypeds(getUntypeds());
    printf("Got %i available untypeds\n", env->num_untypeds);

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
    spawnApp(&initProcess, "init", NULL);

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

static void sel4test_exit(int code)
{
    seL4_TCB_Suspend(seL4_CapInitThreadTCB);
}

int main(void)
{
    KernelTaskContext* env = getKernelTaskContext();
    /* Set exit handler */
    sel4runtime_set_exit(sel4test_exit);
    memset(env, 0, sizeof(KernelTaskContext));
    int error;
    seL4_BootInfo *info = platsupport_get_bootinfo();

#ifdef CONFIG_DEBUG_BUILD
    seL4_DebugNameThread(seL4_CapInitThreadTCB, "kernel_task");
#endif

    /* initialise libsel4simple, which abstracts away which kernel version
     * we are running on */
    simple_default_init_bootinfo(&env->simple, info);

    /* initialise the test environment - allocator, cspace manager, vspace
     * manager, timer
     */
    init_env();

    /* Partially overwrite part of the VKA implementation to cache objects. We need to
     * create this wrapper as the actual vka implementation will only
     * allocate/return any given device frame once.
     * We allocate serial objects for initialising the serial server in
     * platsupport_serial_setup_simple but then we also need to use the objects
     * in some of the tests but attempts to allocate will fail.
     * Instead, this wrapper records the initial serial object allocations and
     * then returns a copy of the one already allocated for future allocations.
     * This requires the allocations for the serial driver to exist for the full
     * lifetime of this application, and for the resources that are allocated to
     * be able to be copied, I.E. frames.
     */
/*
    vka_utspace_alloc_at_base = env->vka.utspace_alloc_at;
    env->vka.utspace_alloc_at = serial_utspace_alloc_at_fn;

    serial_utspace_record = true;
    platsupport_serial_setup_simple(&env->vspace, &env->simple, &env->vka);
    serial_utspace_record = false;
*/
    /* Partially overwrite the IRQ interface so that we can record the IRQ caps that were allocated.
     * We need this only for the timer as the ltimer interfaces allocates the caps for us and hides them away.
     * A few of the tests require actual interactions with the caps hence we record them.
     */

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

