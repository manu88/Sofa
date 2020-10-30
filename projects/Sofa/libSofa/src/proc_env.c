#include "Sofa.h"
#include "sys_calls.h"
#include "proc_ctx.h"
#include <sel4/sel4.h>
#include <stddef.h>
#include <stdio.h>
#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include <sel4utils/vspace.h>
#include <sel4utils/process.h>

#include <sel4utils/thread.h>
#include <sel4utils/thread_config.h>
#include <vka/ipcbuffer.h>
#include <vka/capops.h>

/* dummy global for libsel4muslcsys */
char _cpio_archive[1];
char _cpio_archive_end[1];


/* allocator static pool */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 20)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 4000)

/* global static memory for init */
static sel4utils_alloc_data_t alloc_data;


ProcessContext* _ctx = NULL;

static TLSContext _mainThreadTLS = {0};


static void init_allocator(ProcessContext* context);
static void init_simple(ProcessContext* context);

static seL4_Word get_free_slot(ProcessContext* context);
static seL4_CPtr badge_endpoint(ProcessContext* context, seL4_Word badge, seL4_CPtr ep);

ProcessContext* getProcessContext()
{
    return _ctx;
}

void Thread_init_tls(void *thread)
{
    seL4_SetUserData((seL4_Word)thread);
}

static size_t _do_write_buf(seL4_CPtr endpoint, void *data, size_t count)
{
    const size_t dataSize = count < IPC_BUF_LEN ? count : IPC_BUF_LEN;
    strncpy(_ctx->ipcBuffer, data, dataSize);

    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
    seL4_SetMR(0, SofaSysCall_Write);
    seL4_SetMR(1, dataSize);
    seL4_Send(endpoint, msg);

    return dataSize;
}

static size_t write_buf(void *data, size_t count)
{
    TLSContext* ctx = (TLSContext*)seL4_GetUserData();
    assert(ctx);
    return _do_write_buf(ctx->endpoint, data, count);
}


static ProcessContext* sendInit()
{
    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
    seL4_SetMR(0, SofaSysCall_InitProc);
    seL4_Call(_mainThreadTLS.endpoint, msg);

    ProcessContext* ctx = (ProcessContext*) seL4_GetMR(1);
    return ctx;
}

static void process_exit(int code)
{
    TLSContext* ctx = (TLSContext*)seL4_GetUserData();

    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
    seL4_SetMR(0, SofaSysCall_Exit);
    seL4_SetMR(1, code);
    seL4_Call(ctx->endpoint, msg);
    assert(0);
}

int ProcessInit(seL4_CPtr endpoint)
{
    _mainThreadTLS.endpoint = endpoint;

    _ctx = sendInit();
    Thread_init_tls(&_mainThreadTLS);

    sel4muslcsys_register_stdio_write_fn(write_buf);
    sel4runtime_set_exit(process_exit);
    init_allocator(_ctx);
    init_simple(_ctx);
    //printf("Test thread\n");
    //test_thread();

    return 0;
}

static uint8_t cnode_size_bits(void *data)
{
    ProcessContext *init = (ProcessContext *) data;
    return init->cspace_size_bits;
}

static seL4_CPtr sched_ctrl(void *data, int core)
{
    return ((ProcessContext *) data)->sched_ctrl + core;
}

static int core_count(void *data)
{
    return ((ProcessContext *) data)->cores;
}

static void init_simple(ProcessContext* context)
{
    /* minimal simple implementation */
    context->simple.data = (void *) context;
    context->simple.arch_simple.data = (void *) context;
    context->simple.init_cap = sel4utils_process_init_cap;
    context->simple.cnode_size = cnode_size_bits;
    context->simple.sched_ctrl = sched_ctrl;
    context->simple.core_count = core_count;

    //arch_init_simple(env, &env->simple);
}


static void init_allocator(ProcessContext* context)
{
    UNUSED int error;
    UNUSED reservation_t virtual_reservation;

    /* initialise allocator */
    allocman_t *allocator = bootstrap_use_current_1level(context->root_cnode,
                                                         context->cspace_size_bits, context->free_slots.start,
                                                         context->free_slots.end, ALLOCATOR_STATIC_POOL_SIZE,
                                                         allocator_mem_pool);
    if (allocator == NULL) {
        ZF_LOGF("Failed to bootstrap allocator");
    }
    allocman_make_vka(&context->vka, allocator);

    /* fill the allocator with untypeds */
    seL4_CPtr slot;
    unsigned int size_bits_index;
    size_t size_bits;
    cspacepath_t path;
    for (slot = context->untypeds.start, size_bits_index = 0;
         slot <= context->untypeds.end;
         slot++, size_bits_index++) {

        vka_cspace_make_path(&context->vka, slot, &path);
        /* allocman doesn't require the paddr unless we need to ask for phys addresses,
         * which we don't. */
        size_bits = context->untyped_size_bits_list[size_bits_index];
        error = allocman_utspace_add_uts(allocator, 1, &path, &size_bits, NULL,
                                         ALLOCMAN_UT_KERNEL);
        if (error) {
            ZF_LOGF("Failed to add untyped objects to allocator");
        }
    }

    /* add any arch specific objects to the allocator */
    // empty on X86_64
    //arch_init_allocator(env, init_data);

    /* create a vspace */
    void *existing_frames[context->stack_pages + 2];
    existing_frames[0] = (void *) context;
    existing_frames[1] = seL4_GetIPCBuffer();
    assert(context->stack_pages > 0);
    for (int i = 0; i < context->stack_pages; i++) {
        existing_frames[i + 2] = context->stack + (i * PAGE_SIZE_4K);
    }

    error = sel4utils_bootstrap_vspace(&context->vspace, &alloc_data, context->page_directory, &context->vka,
                                       NULL, NULL, existing_frames);

    /* switch the allocator to a virtual memory pool */
    void *vaddr;
    virtual_reservation = vspace_reserve_range(&context->vspace, ALLOCATOR_VIRTUAL_POOL_SIZE,
                                               seL4_AllRights, 1, &vaddr);
    if (virtual_reservation.res == 0) {
        ZF_LOGF("Failed to switch allocator to virtual memory pool");
    }

    bootstrap_configure_virtual_pool(allocator, vaddr, ALLOCATOR_VIRTUAL_POOL_SIZE,
                                     context->page_directory);

}


int cnode_mint(ProcessContext* context, seL4_CPtr src, seL4_CPtr dest, seL4_CapRights_t rights, seL4_Word badge)
{
    cspacepath_t src_path, dest_path;

    vka_cspace_make_path(&context->vka, src, &src_path);
    vka_cspace_make_path(&context->vka, dest, &dest_path);
    return vka_cnode_mint(&dest_path, &src_path, rights, badge);
}

static seL4_CPtr badge_endpoint(ProcessContext* context, seL4_Word badge, seL4_CPtr ep)
{
    seL4_CPtr slot = get_free_slot(context);
    int error = cnode_mint(context, ep, slot, seL4_AllRights, badge);
    assert(error == seL4_NoError);
    return slot;
}

static seL4_Word get_free_slot(ProcessContext* context)
{
    seL4_CPtr slot;
    UNUSED int error = vka_cspace_alloc(&context->vka, &slot);
    assert(!error);
    return slot;
}


static void set_cap_receive_path(ProcessContext* context, seL4_CPtr slot)
{
    cspacepath_t path;
    vka_cspace_make_path(&context->vka, slot, &path);
    return vka_set_cap_receive_path(&path);
}

int cnode_move(ProcessContext* context, seL4_CPtr src, seL4_CPtr dest)
{
    cspacepath_t src_path, dest_path;

    vka_cspace_make_path(&context->vka, src, &src_path);
    vka_cspace_make_path(&context->vka, dest, &dest_path);
    return vka_cnode_move(&dest_path, &src_path);
}

int is_slot_empty(ProcessContext* context, seL4_Word slot)
{
    int error;

    error = cnode_move(context, slot, slot);

    /* cnode_move first check if the destination is empty and raise
     * seL4_DeleteFirst is it is not
     * The is check if the source is empty and raise seL4_FailedLookup if it is
     */
    assert(error == seL4_DeleteFirst || error == seL4_FailedLookup);
    return (error == seL4_FailedLookup);
}





seL4_CPtr RequestCap(int index)
{
    TLSContext* ctx = (TLSContext*)seL4_GetUserData();

    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault,
                                                        0,  // capsUnwrapped
                                                        0,  // extraCaps
                                                        2);

    seL4_CPtr capDest = get_free_slot(_ctx);
    assert(is_slot_empty(_ctx, capDest));

    set_cap_receive_path(_ctx, capDest);
    seL4_SetMR(0, SofaSysCall_TestCap);
    seL4_SetMR(1, index);    
    seL4_Call(ctx->endpoint, msg);

    assert(is_slot_empty(_ctx, capDest) == 0);

    return capDest;
}
