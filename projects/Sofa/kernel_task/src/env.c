#include "env.h"
#include <allocman/allocman.h>
#include <vspace/vspace.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <sel4utils/vspace.h>
#include <sel4platsupport/bootinfo.h>




/* ammount of untyped memory to reserve for the driver (32mb) */
#define KERN_TASK_UNTYPED_MEMORY (1 << 25)

/* Number of untypeds to try and use to allocate the driver memory.
 * if we cannot get 32mb with 16 untypeds then something is probably wrong */
#define KERN_TASK_NUM_UNTYPEDS 16

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 100)

/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 20)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* static memory for virtual memory bootstrapping */
static sel4utils_alloc_data_t data;

void Environ_init(Environ* env)
{
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
static void free_objects(Environ* env, vka_object_t *objects, unsigned int num)
{
    for (unsigned int i = 0; i < num; i++) {
        vka_free_object(&env->vka, &objects[i]);
    }
}

/* Allocate untypeds till either a certain number of bytes is allocated
 * or a certain number of untyped objects */
static unsigned int allocate_untypeds(Environ* env, vka_object_t *untypeds, size_t bytes, unsigned int max_untypeds)
{
    unsigned int num_untypeds = 0;
    size_t allocated = 0;

    /* try to allocate as many of each possible untyped size as possible */
    for (uint8_t size_bits = seL4_WordBits - 1; size_bits > PAGE_BITS_4K; size_bits--) {
        /* keep allocating until we run out, or if allocating would
         * cause us to allocate too much memory*/
        while (num_untypeds < max_untypeds &&
               allocated + BIT(size_bits) <= bytes &&
               vka_alloc_untyped(&env->vka, size_bits, &untypeds[num_untypeds]) == 0) {
            allocated += BIT(size_bits);
            num_untypeds++;
        }
    }
    return num_untypeds;
}


/* extract a large number of untypeds from the allocator */
unsigned int Environ_populate_untypeds(Environ* env, vka_object_t *untypeds, uint8_t* untyped_size_bits_list)
{
    /* First reserve some memory for the driver */
    vka_object_t reserve[KERN_TASK_NUM_UNTYPEDS];
    unsigned int reserve_num = allocate_untypeds(env, reserve, KERN_TASK_UNTYPED_MEMORY, KERN_TASK_NUM_UNTYPEDS);

    /* Now allocate everything else for the tests */
    unsigned int num_untypeds = allocate_untypeds(env, untypeds, UINT_MAX, ARRAY_SIZE(untyped_size_bits_list));
    /* Fill out the size_bits list */
    for (unsigned int i = 0; i < num_untypeds; i++) {
        untyped_size_bits_list[i] = untypeds[i].size_bits;
    }

    /* Return reserve memory */
    free_objects(env, reserve, reserve_num);

    /* Return number of untypeds for tests */
    if (num_untypeds == 0) {
        ZF_LOGF("No untypeds for tests!");
    }

    return num_untypeds;
}