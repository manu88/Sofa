#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include "test_init_data.h"
#include "test.h"
#include "helpers.h"

static seL4_CPtr endpoint;
static struct env env;


/* global static memory for init */
static sel4utils_alloc_data_t alloc_data;

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 4000)

/* allocator static pool */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 20)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];



static void init_allocator(env_t env, test_init_data_t *init_data)
{
    UNUSED int error;
    UNUSED reservation_t virtual_reservation;

    /* initialise allocator */
    allocman_t *allocator = bootstrap_use_current_1level(init_data->root_cnode,
                                                         init_data->cspace_size_bits, init_data->free_slots.start,
                                                         init_data->free_slots.end, ALLOCATOR_STATIC_POOL_SIZE,
                                                         allocator_mem_pool);
    if (allocator == NULL) {
        ZF_LOGF("Failed to bootstrap allocator");
    }
    allocman_make_vka(&env->vka, allocator);

    /* fill the allocator with untypeds */
    seL4_CPtr slot;
    unsigned int size_bits_index;
    size_t size_bits;
    cspacepath_t path;
    for (slot = init_data->untypeds.start, size_bits_index = 0;
         slot <= init_data->untypeds.end;
         slot++, size_bits_index++) {

        vka_cspace_make_path(&env->vka, slot, &path);
        /* allocman doesn't require the paddr unless we need to ask for phys addresses,
         * which we don't. */
        size_bits = init_data->untyped_size_bits_list[size_bits_index];
        error = allocman_utspace_add_uts(allocator, 1, &path, &size_bits, NULL,
                                         ALLOCMAN_UT_KERNEL);
        if (error) {
            ZF_LOGF("Failed to add untyped objects to allocator");
        }
    }

    /* add any arch specific objects to the allocator */
    // Empty for X86 : arch_init_allocator(env, init_data);

    /* create a vspace */
    void *existing_frames[init_data->stack_pages + 2];
    existing_frames[0] = (void *) init_data;
    existing_frames[1] = seL4_GetIPCBuffer();
    assert(init_data->stack_pages > 0);
    for (int i = 0; i < init_data->stack_pages; i++) {
        existing_frames[i + 2] = init_data->stack + (i * PAGE_SIZE_4K);
    }

    error = sel4utils_bootstrap_vspace(&env->vspace, &alloc_data, init_data->page_directory, &env->vka,
                                       NULL, NULL, existing_frames);

    /* switch the allocator to a virtual memory pool */
    void *vaddr;
    virtual_reservation = vspace_reserve_range(&env->vspace, ALLOCATOR_VIRTUAL_POOL_SIZE,
                                               seL4_AllRights, 1, &vaddr);
    if (virtual_reservation.res == 0) {
        ZF_LOGF("Failed to switch allocator to virtual memory pool");
    }

    bootstrap_configure_virtual_pool(allocator, vaddr, ALLOCATOR_VIRTUAL_POOL_SIZE,
                                     env->page_directory);

}


static uint8_t cnode_size_bits(void *data)
{
    test_init_data_t *init = (test_init_data_t *) data;
    return init->cspace_size_bits;
}

static seL4_CPtr sched_ctrl(void *data, int core)
{
    return ((test_init_data_t *) data)->sched_ctrl + core;
}

static int core_count(UNUSED void *data)
{
    return ((test_init_data_t *) data)->cores;
}

void init_simple(env_t env, test_init_data_t *init_data)
{
    /* minimal simple implementation */
    env->simple.data = (void *) init_data;
    env->simple.arch_simple.data = (void *) init_data;
    env->simple.init_cap = sel4utils_process_init_cap;
    env->simple.cnode_size = cnode_size_bits;
    env->simple.sched_ctrl = sched_ctrl;
    env->simple.core_count = core_count;

    //arch_init_simple(env, &env->simple);
}

static int thread1(seL4_Word ep, seL4_Word id, seL4_Word runs, seL4_Word arg3);


static seL4_CPtr badge_endpoint(env_t env, seL4_Word badge, seL4_CPtr ep)
{
    seL4_CPtr slot = get_free_slot(env);
    int error = cnode_mint(env, ep, slot, seL4_AllRights, badge);
    assert(error == seL4_NoError);
    return slot;
}

int main(int argc, char *argv[])
{
    test_init_data_t *init_data;
    

/* parse args */
    assert(argc == 2);
    endpoint = (seL4_CPtr) atoi(argv[0]);

/* read in init data */
    init_data = (void *) atol(argv[1]);


/* configure env */
    env.cspace_root = init_data->root_cnode;
    env.page_directory = init_data->page_directory;
    env.endpoint = endpoint;
    env.priority = init_data->priority;
    env.cspace_size_bits = init_data->cspace_size_bits;
    env.tcb = init_data->tcb;
    env.domain = init_data->domain;
    env.asid_pool = init_data->asid_pool;
    env.asid_ctrl = init_data->asid_ctrl;
    env.sched_ctrl = init_data->sched_ctrl;
#ifdef CONFIG_IOMMU
    env.io_space = init_data->io_space;
#endif
#ifdef CONFIG_TK1_SMMU
    env.io_space_caps = init_data->io_space_caps;
#endif
    env.cores = init_data->cores;
    env.timer_notification.cptr = init_data->timer_ntfn;

    env.device_frame = init_data->device_frame_cap;

/* initialise cspace, vspace and untyped memory allocation */
    init_allocator(&env, init_data);

/* initialise simple */
    init_simple(&env, init_data);

   


    seL4_CPtr sync_ep = vka_alloc_endpoint_leaky(&env.vka);
    seL4_CPtr badged_sync_ep = badge_endpoint(&env, 10, sync_ep);
    helper_thread_t sync;
    create_helper_thread(&env, &sync);
    start_helper(&env, &sync, thread1, badged_sync_ep, 0, 0, 0);

    wait_for_helper(&sync);
    printf("Thread returned\n");

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 42);
    seL4_Send(endpoint, info);

    while (1)
    {
        /* code */
    }
    
    
    return 0;
}


static int thread1(seL4_Word ep, seL4_Word id, seL4_Word runs, seL4_Word arg3)
{
    return 0;
    int acc = 10;
    while(acc--)
    {
        seL4_MessageInfo_t info = seL4_MessageInfo_new(0, 0, 0, 1);
        seL4_SetMR(0, acc);
        seL4_Send((seL4_CPtr) ep, info);
    }
}
