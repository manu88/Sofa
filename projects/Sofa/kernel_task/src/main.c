#include <stdio.h>
#include <cpio/cpio.h>
#include <sel4platsupport/bootinfo.h>
#include <simple-default/simple-default.h>
#include <assert.h>
#include <sel4utils/stack.h>

#include <sel4utils/process.h>
#include <vka/capops.h>
#include <cpio/cpio.h>

//#include <sel4platsupport/arch/io.h>
#include <sel4platsupport/device.h>
#include <sys_calls.h>
#include <proc_ctx.h>

#include "env.h"
#include "timer.h"
#include "utils.h"
#include "Process.h"

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)


/* list of untypeds to give out to processes */
static vka_object_t untypeds[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];
/* list of sizes (in bits) corresponding to untyped */
static uint8_t untyped_size_bits_list[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];

static Environ _envir;

static Process initProcess;
static Process timeServer;

extern char _cpio_archive[];
extern char _cpio_archive_end[];

static ps_irq_register_fn_t irq_register_fn_copy;

void *main_continued(void *arg UNUSED);


static irq_id_t sel4test_timer_irq_register(UNUSED void *cookie, ps_irq_t irq, irq_callback_fn_t callback,
                                            void *callback_data)
{
    printf("--> sel4test_timer_irq_register called \n");
    static int num_timer_irqs = 0;

    int error;

    ZF_LOGF_IF(!callback, "Passed in a NULL callback");

    ZF_LOGF_IF(num_timer_irqs >= MAX_TIMER_IRQS, "Trying to register too many timer IRQs");

    /* Allocate the IRQ */
    error = sel4platsupport_copy_irq_cap(&_envir.vka, &_envir.simple, &irq,
                                         &_envir.timer_irqs[num_timer_irqs].handler_path);
    ZF_LOGF_IF(error, "Failed to allocate IRQ handler");

    /* Allocate the root notifitcation if we haven't already done so */
    if (_envir.timer_notification.cptr == seL4_CapNull) {
        error = vka_alloc_notification(&_envir.vka, &_envir.timer_notification);
        ZF_LOGF_IF(error, "Failed to allocate notification object");
    }
    assert(_envir.timer_notification.cptr != seL4_CapNull);

    /* Mint a notification for the IRQ handler to pair with */
    error = vka_cspace_alloc_path(&_envir.vka, &_envir.badged_timer_notifications[num_timer_irqs]);
    ZF_LOGF_IF(error, "Failed to allocate path for the badged notification");
    cspacepath_t root_notification_path = {0};
    vka_cspace_make_path(&_envir.vka, _envir.timer_notification.cptr, &root_notification_path);
    error = vka_cnode_mint(&_envir.badged_timer_notifications[num_timer_irqs],
                           &root_notification_path,
                           seL4_AllRights, IRQ_EP_BADGE);//BIT(num_timer_irqs));

    ZF_LOGF_IF(error, "Failed to mint notification for timer");

    /* Pair the notification and the handler */
    error = seL4_IRQHandler_SetNotification(_envir.timer_irqs[num_timer_irqs].handler_path.capPtr,
                                            _envir.badged_timer_notifications[num_timer_irqs].capPtr);

    ZF_LOGF_IF(error, "Failed to pair the notification and handler together");

    /* Ack the handler so interrupts can come in */
    error = seL4_IRQHandler_Ack(_envir.timer_irqs[num_timer_irqs].handler_path.capPtr);
    ZF_LOGF_IF(error, "Failed to ack the IRQ handler");

    /* Fill out information about the callbacks */
    //_envir.timer_cbs[num_timer_irqs].callback = callback;
    //_envir.timer_cbs[num_timer_irqs].callback_data = callback_data;

    return num_timer_irqs++;
}

int main(void)
{
    int error;
    seL4_BootInfo *bootInfo = platsupport_get_bootinfo();
    simple_default_init_bootinfo(&_envir.simple, bootInfo);

    printf("\n\n");
    printf("-> Init environment\n");
    Environ_init(&_envir);
    
    printf("-> init kernel_task endpoint\n");
    error = vka_alloc_endpoint( &_envir.vka, &_envir.rootTaskEP);
    ZF_LOGF_IF(error, "Failed to alloc kernel_task endpoint");

    printf("-> Init timer\n");

    irq_register_fn_copy = _envir.ops.irq_ops.irq_register_fn;
    _envir.ops.irq_ops.irq_register_fn = sel4test_timer_irq_register;
    Timer_init(&_envir);
    _envir.ops.irq_ops.irq_register_fn = irq_register_fn_copy;


    fflush(stdout);
    void *res;

    error = sel4utils_run_on_stack(&_envir.vspace, main_continued, NULL, &res);
    assert(error == 0);
    assert(res == 0);
    
    return 0;
}


void listCPIOFiles()
{
    unsigned long cpioArchiveLen = _cpio_archive_end - _cpio_archive;
    struct cpio_info cpioInfos;
    int error;
    error = cpio_info(_cpio_archive, cpioArchiveLen, &cpioInfos);
    ZF_LOGF_IF(error, "Failed to Get CPIO info");

    printf("-> CPIO: got %i file(s)\n", cpioInfos.file_count);

    char **fileNames = malloc(cpioInfos.file_count);
    ZF_LOGF_IF(fileNames == NULL, "Failed to malloc memory for CPIO files");

    for (int i=0;i<cpioInfos.file_count; i++)
    {
        fileNames[i] = malloc(cpioInfos.max_path_sz);
        ZF_LOGF_IF(fileNames[i] == NULL, "Failed to malloc memory for CPIO file at %i", i);
    }

    cpio_ls(_cpio_archive, cpioArchiveLen, fileNames, cpioInfos.file_count);

    for(int i=0;i<cpioInfos.file_count; i++)
    {
        printf("\tfile : '%s'\n", fileNames[i]);
    }

    // cleanup
        for(int i=0;i<cpioInfos.file_count; i++)
    {
        free(fileNames[i]);
    }

    free(fileNames);
}

int spawnApp(Process* process, const char* appName)
{
    unsigned long fileSize = 0;
    void* filePos = cpio_get_file(_cpio_archive, _cpio_archive_end - _cpio_archive, appName, &fileSize);

    if(filePos == NULL)
    {
        printf("file '%s' not found\n",appName);
        return -ENOENT;
    }
    int error;

    sel4utils_process_config_t config = process_config_default_simple( &_envir.simple, appName, seL4_MaxPrio-1);


    cspacepath_t badged_ep_path;
    error = vka_cspace_alloc_path(&_envir.vka, &badged_ep_path);
    ZF_LOGF_IFERR(error, "Failed to allocate path\n");
    assert(error == 0);

    cspacepath_t ep_path = {0};
    vka_cspace_make_path(&_envir.vka, _envir.rootTaskEP.cptr, &ep_path);

    process->pid = Process_GetNextPID();
    error = vka_cnode_mint(&badged_ep_path, &ep_path, seL4_AllRights, process->pid);
    assert(error == 0);

    config = process_config_fault_cptr(config ,badged_ep_path.capPtr );

    error = sel4utils_configure_process_custom(&process->_process , &_envir.vka , &_envir.vspace, config);
    ZF_LOGF_IFERR(error, "Failed to configure a new process.\n");
    assert(error == 0);

    process->endpoint = process_copy_cap_into(&process->_process, process->pid, &_envir.vka, _envir.rootTaskEP.cptr, seL4_AllRights);
    char endpoint_string[16] = "";
    snprintf(endpoint_string, 16, "%ld", (long) process->endpoint);// process_ep_cap);

    seL4_Word argc = 2;
    char *argv[] = { (char*)appName, endpoint_string};

    error = sel4utils_spawn_process_v(&process->_process , &_envir.vka , &_envir.vspace , argc, argv , 1);
    ZF_LOGF_IFERR(error, "Failed to spawn and start the new thread.\n");

    printf("Started app '%s'\n", appName);
    return 0;
}

void ProcessContext_init(ProcessContext* ctx)
{
    memset(ctx, 0, sizeof(ProcessContext));
    ctx->root_cnode = SEL4UTILS_CNODE_SLOT;
    ctx->cspace_size_bits = SOFA_PROCESS_CSPACE_SIZE_BITS;
    ctx->stack_pages = CONFIG_SEL4UTILS_STACK_SIZE / PAGE_SIZE_4K;
    ctx->cores = simple_get_core_count(&_envir.simple);
}


static seL4_SlotRegion copy_untypeds_to_process(sel4utils_process_t *process, vka_object_t *untypeds, int num_untypeds,
                                                Environ* env)
{
    seL4_SlotRegion range = {0};

    for (int i = 0; i < num_untypeds; i++) {
        seL4_CPtr slot = sel4utils_copy_cap_to_process(process, &env->vka, untypeds[i].cptr);

        /* set up the cap range */
        if (i == 0) {
            range.start = slot;
        }
        range.end = slot;
    }
    assert((range.end - range.start) + 1 == num_untypeds);
    return range;
}

void on_initCall(Process* process, seL4_MessageInfo_t message)
{
    assert(process->ctx == NULL);
    ProcessContext* ctx = (ProcessContext *) vspace_new_pages(&_envir.vspace, seL4_AllRights, 1, PAGE_BITS_4K);
    assert(ctx != NULL);
    process->ctx = ctx;
    ProcessContext_init(ctx);

    ctx->pid = process->pid;

    seL4_CPtr process_data_frame = vspace_get_cap(&_envir.vspace, ctx);
    seL4_CPtr process_data_frame_copy = 0;
    util_copy_cap(&_envir.vka, process_data_frame, &process_data_frame_copy);

    size_t numPages = 1;
    void* venv = vspace_map_pages(&process->_process.vspace, &process_data_frame_copy, NULL, seL4_AllRights, numPages, PAGE_BITS_4K, 1/*cacheable*/);

    const size_t numUntypedsPerProcess = 4;//_envir.num_untypeds / 8;
    printf("Allocate %lu untypeds for process %i\n", numUntypedsPerProcess, process->pid);
    printf("Index is %i\n", _envir.index_in_untyped);
    memcpy(ctx->untyped_size_bits_list,
           untyped_size_bits_list + _envir.index_in_untyped,
           sizeof(uint8_t) * numUntypedsPerProcess);

    ctx->page_directory = sel4utils_copy_cap_to_process(&process->_process, &_envir.vka, process->_process.pd.cptr);
    ctx->tcb = sel4utils_copy_cap_to_process(&process->_process, &_envir.vka, process->_process.thread.tcb.cptr);

// set priority for process other threads
    seL4_Error err =  seL4_TCB_SetMCPriority(process->_process.thread.tcb.cptr, seL4_CapInitThreadTCB, 254);
    if(err != seL4_NoError)
    {
        printf("seL4_TCB_SetMCPriority (1) err %i\n", err);
    }
// end set priority for process other threads

    /* setup data about untypeds */
    ctx->untypeds = copy_untypeds_to_process(&process->_process,
                                             _envir.untypeds + _envir.index_in_untyped,
                                             numUntypedsPerProcess,
                                             &_envir);
    /* WARNING: DO NOT COPY MORE CAPS TO THE PROCESS BEYOND THIS POINT,
     * AS THE SLOTS WILL BE CONSIDERED FREE AND OVERRIDDEN BY THE TEST PROCESS. */
    /* set up free slot range */

    ctx->free_slots.start =  ctx->untypeds.end + 1;
    ctx->free_slots.end = (1u << SOFA_PROCESS_CSPACE_SIZE_BITS);
    assert(ctx->free_slots.start < ctx->free_slots.end);

    seL4_SetMR(1, (seL4_Word) venv);
    seL4_Reply(message);

    _envir.index_in_untyped += numUntypedsPerProcess;
}

void cleanup_process(Process* proc)
{
    vspace_unmap_pages(&_envir.vspace, proc->ctx, 1, PAGE_BITS_4K, NULL);
    sel4utils_destroy_process(&proc->_process, &_envir.vka);
}

static void printSpecs()
{
    printf("Kernel Build %s MCS\n", config_set(CONFIG_KERNEL_MCS)? "with": "without");    
}

int process_spawn(Process* callingProcess, seL4_MessageInfo_t message)
{
    const size_t filePathLen = (size_t) seL4_GetMR(1);
    printf("Spawn request from %i: '%s' \n", callingProcess->pid, callingProcess->ctx->ipcBuffer);
    const char* filePath = (const char*) callingProcess->ctx->ipcBuffer;

    Process* p = malloc(sizeof(Process));
    if(p == NULL)
    {
        return -ENOMEM;
    }
    ProcessInit(p);
    int error = spawnApp(p, filePath);

    if (error != 0)
    {
        free(p);
        return error;
    }
    Process_Add(p);

    return p->pid;
}

void *main_continued(void *arg UNUSED)
{
    printf("\n------Sofa------\n");

    printSpecs();

    /* allocate lots of untyped memory for tests to use */

    _envir.num_untypeds = Environ_populate_untypeds(&_envir, untypeds, untyped_size_bits_list, ARRAY_SIZE(untyped_size_bits_list));
    printf("Allocated %i untypeds  %i \n", _envir.num_untypeds, simple_get_untyped_count(&_envir.simple));
    _envir.untypeds = untypeds;

    listCPIOFiles();

    ProcessInit(&initProcess);
    spawnApp(&initProcess, "init");
    printf("Add process with pid %i\n", initProcess.pid);
    Process_Add(&initProcess);

    seL4_DebugDumpScheduler();

// timer test
#if 0
    unsigned int timerID = 0;
    int err = tm_alloc_id(&_envir.tm, &timerID);
    assert(err == 0);
    printf("Timer ID is %u\n", timerID);
    tm_register_rel_cb(&_envir.tm, 5*NS_IN_S, timerID, on_time, 1);
#endif
// timer test

    printf("-> start kernel_task run loop\n");
    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t message = seL4_Recv(_envir.rootTaskEP.cptr, &sender);
        seL4_Word label = seL4_MessageInfo_get_label(message);

        if(sender & IRQ_EP_BADGE)
        {           
            int error = seL4_IRQHandler_Ack(_envir.timer_irqs[0].handler_path.capPtr);
            ZF_LOGF_IF(error, "Failed to acknowledge timer IRQ handler");

            tm_update(&_envir.tm);
        }
        else if(label == seL4_NoFault)
        {
            seL4_Word rpcID = seL4_GetMR(0);
            Process* callingProcess = Process_GetByPID(sender);
            if(callingProcess == NULL)
            {
                printf("Process not found for %lu\n", sender);
            }
            assert(callingProcess);
            if (rpcID == SofaSysCall_InitProc)
            {
                printf("Received init call from %lu\n", sender);
                on_initCall(callingProcess, message);

            }
            else if (rpcID == SofaSysCall_Exit)
            {
                printf("exit from %i\n", callingProcess->pid);
                Process_Remove(callingProcess);
                cleanup_process(callingProcess);
                seL4_DebugDumpScheduler();
            }
            else if (rpcID == SofaSysCall_Debug)
            {
                seL4_DebugDumpScheduler();
            }
            else if (rpcID == SofaSysCall_Write)
            {
                const size_t dataSize = (size_t) seL4_GetMR(1);
                for(size_t i =0; i< dataSize;i++)
                {
                    putchar(callingProcess->ctx->ipcBuffer[i]);
                }
            }
            else if (rpcID == SofaSysCall_Spawn)
            {
                int ret = process_spawn(callingProcess, message);
                int pid = 0;
                if(ret > 0)
                {
                    pid = ret;
                    ret = 0;
                }
                else
                {
                    ret = - ret;
                }
                
                seL4_SetMR(1, ret);
                seL4_SetMR(2, pid);
                seL4_Reply(message);
            }

            else
            {
                printf("Received unknown RPC id %lu from sender %lu\n", rpcID, sender);
            }
            
        }
        else if( label == seL4_CapFault)
        {
            Process* callingProcess = Process_GetByPID(sender);
            printf("Got cap fault from %i\n", callingProcess->pid);
        }
        else if (label == seL4_VMFault)
        {
            Process* callingProcess = Process_GetByPID(sender);
            printf("Got VM fault from %i\n", callingProcess->pid);
            const seL4_Word programCounter      = seL4_GetMR(seL4_VMFault_IP);
            const seL4_Word faultAddr           = seL4_GetMR(seL4_VMFault_Addr);
            const seL4_Word isPrefetch          = seL4_GetMR(seL4_VMFault_PrefetchFault);
            const seL4_Word faultStatusRegister = seL4_GetMR(seL4_VMFault_FSR);
            
            printf("[kernel_task] programCounter      %lu\n",programCounter);
            printf("[kernel_task] faultAddr           %lu\n",faultAddr);
            printf("[kernel_task] isPrefetch          %lu\n",isPrefetch);
            printf("[kernel_task] faultStatusRegister 0X%lX\n",faultStatusRegister);
            seL4_DebugDumpScheduler();
        }
        else if(label == seL4_UserException)
        {
            printf("got user exception\n");
        }
        else
        {
            printf("got Other msg label %lx\n" , label);
        }
    }
    
}