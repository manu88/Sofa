#include <stdio.h>
#include <cpio/cpio.h>
#include <sel4platsupport/bootinfo.h>
#include <simple-default/simple-default.h>
#include <assert.h>
#include <sel4utils/stack.h>

#include <sel4utils/process.h>
#include <vka/capops.h>
#include <cpio/cpio.h>

#include <sys_calls.h>
#include <proc_ctx.h>

#include "env.h"
#include "timer.h"
#include "utils.h"
#include "Process.h"




/* list of untypeds to give out to processes */
static vka_object_t untypeds[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];
/* list of sizes (in bits) corresponding to untyped */
static uint8_t untyped_size_bits_list[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];

static Environ _envir;

static Process testProcess;

extern char _cpio_archive[];
extern char _cpio_archive_end[];


void *main_continued(void *arg UNUSED);

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
    Timer_init(&_envir);

    fflush(stdout);
    void *res;

    /* Run sel4test-test related tests */
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



void spawnApp(Process* process, const char* appName, seL4_Word badge)
{
    int error;

    sel4utils_process_config_t config = process_config_default_simple( &_envir.simple, appName, seL4_MaxPrio-1);


    cspacepath_t badged_ep_path;
    error = vka_cspace_alloc_path(&_envir.vka, &badged_ep_path);
    ZF_LOGF_IFERR(error, "Failed to allocate path\n");
    

    cspacepath_t ep_path = {0};
    vka_cspace_make_path(&_envir.vka, _envir.rootTaskEP.cptr, &ep_path);

    
    error = vka_cnode_mint(&badged_ep_path, &ep_path, seL4_AllRights, 1);

    config = process_config_fault_cptr(config ,badged_ep_path.capPtr );

    error = sel4utils_configure_process_custom(&process->_process , &_envir.vka , &_envir.vspace, config);
    ZF_LOGF_IFERR(error, "Failed to configure a new process.\n");

    process->endpoint = process_copy_cap_into(&process->_process, badge, &_envir.vka, _envir.rootTaskEP.cptr, seL4_AllRights);
    char endpoint_string[16] = "";
    snprintf(endpoint_string, 16, "%ld", (long) process->endpoint);// process_ep_cap);

    seL4_Word argc = 2;
    char *argv[] = { (char*)appName, endpoint_string};

    error = sel4utils_spawn_process_v(&process->_process , &_envir.vka , &_envir.vspace , argc, argv , 1);
    ZF_LOGF_IFERR(error, "Failed to spawn and start the new thread.\n");

    printf("Started app '%s'\n", appName);
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
    ProcessContext* ctx = (ProcessContext *) vspace_new_pages(&_envir.vspace, seL4_AllRights, 1, PAGE_BITS_4K);
    assert(ctx != NULL);
    process->ctx = ctx;
    ProcessContext_init(ctx);


    seL4_CPtr process_data_frame = vspace_get_cap(&_envir.vspace, ctx);
    seL4_CPtr process_data_frame_copy = 0;
    util_copy_cap(&_envir.vka, process_data_frame, &process_data_frame_copy);

    size_t numPages = 1;
    void* venv = vspace_map_pages(&process->_process.vspace, &process_data_frame_copy, NULL, seL4_AllRights, numPages, PAGE_BITS_4K, 1/*cacheable*/);

    memcpy(ctx->untyped_size_bits_list, untyped_size_bits_list, sizeof(uint8_t) * _envir.num_untypeds);

    ctx->page_directory = sel4utils_copy_cap_to_process(&process->_process, &_envir.vka, process->_process.pd.cptr);
    ctx->tcb = sel4utils_copy_cap_to_process(&process->_process, &_envir.vka, process->_process.thread.tcb.cptr);

    seL4_Error err =  seL4_TCB_SetMCPriority(process->_process.thread.tcb.cptr, seL4_CapInitThreadTCB, 254);
    if(err != seL4_NoError)
    {
        printf("seL4_TCB_SetMCPriority (1) err %i\n", err);
    }

    /* setup data about untypeds */
    ctx->untypeds = copy_untypeds_to_process(&process->_process, _envir.untypeds, _envir.num_untypeds, &_envir);
    /* WARNING: DO NOT COPY MORE CAPS TO THE PROCESS BEYOND THIS POINT,
     * AS THE SLOTS WILL BE CONSIDERED FREE AND OVERRIDDEN BY THE TEST PROCESS. */
    /* set up free slot range */

    ctx->free_slots.start =  ctx->untypeds.end + 1;
    ctx->free_slots.end = (1u << SOFA_PROCESS_CSPACE_SIZE_BITS);
    assert(ctx->free_slots.start < ctx->free_slots.end);

    seL4_SetMR(1, (seL4_Word) venv);
    seL4_Reply(message);
}


static void printSpecs()
{
    printf("Kernel Build %s MCS\n", config_set(CONFIG_KERNEL_MCS)? "with": "without");    
}

void *main_continued(void *arg UNUSED)
{
    printf("\n------Sofa------\n");

    printSpecs();

    /* allocate lots of untyped memory for tests to use */
    _envir.num_untypeds = Environ_populate_untypeds(&_envir, untypeds, untyped_size_bits_list);
    printf("Allocated %i untypeds \n", _envir.num_untypeds);
    _envir.untypeds = untypeds;

    listCPIOFiles();
    spawnApp(&testProcess, "app", 1);

    seL4_DebugDumpScheduler();

    printf("-> start kernel_task run loop\n");
    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t message = seL4_Recv(_envir.rootTaskEP.cptr, &sender);
        seL4_Word label = seL4_MessageInfo_get_label(message);
        //printf("Received something from %lu\n", sender);
        if(label == seL4_NoFault)
        {
            seL4_Word rpcID = seL4_GetMR(0);
            if (rpcID == SofaSysCall_Write)
            {
                const size_t dataSize = (size_t) seL4_GetMR(1);
                for(size_t i =0; i< dataSize;i++)
                {
                    putchar(testProcess.ctx->ipcBuffer[i]);
                }
            }
            else if (rpcID == SofaSysCall_InitProc)
            {
                printf("Received init call from %lu\n", sender);
                on_initCall(&testProcess, message);

            }
            else if (rpcID == SofaSysCall_Debug)
            {
                seL4_DebugDumpScheduler();
            }
            else
            {
                printf("Received unkown RPC id %lu\n", rpcID);
            }
            
        }
        else if( label == seL4_CapFault)
        {
            printf("Got cap fault\n");
        }
        else if (label == seL4_VMFault)
        {
            printf("Got VM fault\n");
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