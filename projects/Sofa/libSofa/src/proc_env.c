#include "Sofa.h"
#include "sys_calls.h"
#include "proc_ctx.h"
#include <sel4/sel4.h>
#include <stddef.h>
#include <stdio.h>
#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include <sel4utils/vspace.h>

/* allocator static pool */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 20)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 4000)

/* global static memory for init */
static sel4utils_alloc_data_t alloc_data;



static seL4_CPtr _endpoint = 0;
static ProcessContext* _ctx = NULL;



static void init_allocator(ProcessContext* context);

static size_t write_buf(void *data, size_t count)
{
    if(_endpoint == 0)
    {
        return 0;
    }
    for (int i=0;i<count ; i++)
    {
        struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
        seL4_SetMR(0, SofaSysCall_PutChar);
        seL4_SetMR(1, ((char*)data)[i]);
        seL4_Send(_endpoint, msg);
    }
    return count;
}

static ProcessContext* sendInit()
{
    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
    seL4_SetMR(0, SofaSysCall_InitProc);
    seL4_Call(_endpoint, msg);

    ProcessContext* ctx = seL4_GetMR(1);
    return ctx;
}

void dump_ProcessContext()
{
    printf("ProcessContext.page_directory %ld\n",  _ctx->page_directory);
    printf("ProcessContext.root_cnode %ld\n", _ctx->root_cnode);
    printf("ProcessContext.tcb %ld\n", _ctx->tcb);
    printf("ProcessContext.stack %ld\n", _ctx->stack);

}
int ProcessInit(void* endpoint)
{
    _endpoint = endpoint;

    sel4muslcsys_register_stdio_write_fn(write_buf);

    printf("Send init RPC call\n");
    _ctx = sendInit();

    init_allocator(_ctx);
    printf("Allocator ok\n");
    dump_ProcessContext();

    return 0;
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

    printf("fill the allocator with untypeds\n");
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

    printf("create vspace\n");
    /* create a vspace */
    void *existing_frames[context->stack_pages + 2];
    existing_frames[0] = (void *) context;
    existing_frames[1] = seL4_GetIPCBuffer();
    assert(context->stack_pages > 0);
    for (int i = 0; i < context->stack_pages; i++) {
        existing_frames[i + 2] = context->stack + (i * PAGE_SIZE_4K);
    }

    printf("boostrap vspace\n");
    error = sel4utils_bootstrap_vspace(&context->vspace, &alloc_data, context->page_directory, &context->vka,
                                       NULL, NULL, existing_frames);

    /* switch the allocator to a virtual memory pool */
    void *vaddr;
    virtual_reservation = vspace_reserve_range(&context->vspace, ALLOCATOR_VIRTUAL_POOL_SIZE,
                                               seL4_AllRights, 1, &vaddr);
    if (virtual_reservation.res == 0) {
        ZF_LOGF("Failed to switch allocator to virtual memory pool");
    }

    printf("configure virtual pool\n");
    bootstrap_configure_virtual_pool(allocator, vaddr, ALLOCATOR_VIRTUAL_POOL_SIZE,
                                     context->page_directory);

}