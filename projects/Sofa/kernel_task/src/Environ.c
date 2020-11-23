#include "Environ.h"
#include <allocman/bootstrap.h>
#include <sel4utils/vspace.h>
#include <sel4platsupport/bootinfo.h>
#include <simple-default/simple-default.h>
#include <simple/simple_helpers.h>
#include <allocman/vka.h>
#include <sel4platsupport/io.h>

#include <muslcsys/vsyscall.h>

#if 0
/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE 67108864//((1 << seL4_PageBits) * 1024)
#define SEL4OSAPI_SYSTEM_VMEM_SIZE 67108864
/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE 40960//((1 << seL4_PageBits) * 20)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

#define SEL4OSAPI_BOOTSTRAP_MEM_POOL_SIZE  40960
UNUSED static char malloc_static_mem_pool[SEL4OSAPI_BOOTSTRAP_MEM_POOL_SIZE];


sel4utils_res_t muslc_brk_reservation_memory;

extern vspace_t *muslc_this_vspace;
extern reservation_t muslc_brk_reservation;
extern void *muslc_brk_reservation_start;

extern char *morecore_area;
extern size_t morecore_size;


// TO KEEP

/* static memory for virtual memory bootstrapping */
static sel4utils_alloc_data_t vspace_data;
static char allocator_mempool[8886080];
#define ALLOCMAN_VIRTUAL_SIZE 400000000
#define BRK_VIRTUAL_SIZE ((1 << seL4_PageBits) * 200)
#endif

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 400)


/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 100)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* static memory for virtual memory bootstrapping */
static sel4utils_alloc_data_t data;

static KernelTaskContext _ctx;
uint8_t _env_set = 0;

KernelTaskContext* getKernelTaskContext(void)
{
    return &_ctx;
}


int IOInit()
{
    KernelTaskContext* env = getKernelTaskContext();


    int error = sel4platsupport_get_io_port_ops(&env->ops.io_port_ops, &env->simple, &env->vka);
    assert(error == 0);

    error = sel4utils_new_page_dma_alloc(&env->vka, &env->vspace, &env->ops.dma_manager);
    assert(error == 0);
}

/* globals for malloc */
extern vspace_t *muslc_this_vspace;
extern reservation_t muslc_brk_reservation;
extern void *muslc_brk_reservation_start;
static sel4utils_res_t malloc_res;

static void CONSTRUCTOR(MUSLCSYS_WITH_VSYSCALL_PRIORITY)  init_malloc(void)
{
    KernelTaskContext* env = getKernelTaskContext();
    int error;

    seL4_BootInfo *info = platsupport_get_bootinfo(); // seL4_GetBootInfo();
    simple_default_init_bootinfo(&env->simple, info);

    
/* create allocator */
    env->allocman = bootstrap_use_current_simple(&env->simple, ALLOCATOR_STATIC_POOL_SIZE, allocator_mem_pool);
    assert(env->allocman);

    /* create vka */
    allocman_make_vka(&env->vka, env->allocman);

    /* create vspace */
    error = sel4utils_bootstrap_vspace_with_bootinfo_leaky(&env->vspace, &data, simple_get_pd(&env->simple),
                                                           &env->vka, info);

    /* set up malloc */
    error = sel4utils_reserve_range_no_alloc(&env->vspace, &malloc_res, seL4_LargePageBits, seL4_AllRights, 1,
                                             &muslc_brk_reservation_start);
    muslc_this_vspace = &env->vspace;
    muslc_brk_reservation.res = &malloc_res;
    ZF_LOGF_IF(error, "Failed to set up dynamic malloc");

    void *vaddr;
    reservation_t virtual_reservation = vspace_reserve_range(&env->vspace, ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights,
                                               1, &vaddr);
    assert(virtual_reservation.res);
    bootstrap_configure_virtual_pool(env->allocman, vaddr, ALLOCATOR_VIRTUAL_POOL_SIZE, simple_get_pd(&env->simple));
    
    _env_set = 1;

}