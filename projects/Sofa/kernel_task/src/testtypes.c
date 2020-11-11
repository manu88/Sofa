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

#include <sel4debug/register_dump.h>
#include <vka/capops.h>

#include "test.h"
#include "testtypes.h"
#include "utils.h"

#define TIMER_ID 0


/* Basic test type. Each test is launched as its own process. */
/* copy untyped caps into a processes cspace, return the cap range they can be found in */
seL4_SlotRegion copy_untypeds_to_process(sel4utils_process_t *process, vka_object_t *untypeds, int num_untypeds,
                                                driver_env_t env)
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


void basic_set_up(driver_env_t env, Process* process)
{
    int error;

    //root_task_endpoint


    sel4utils_process_config_t config = process_config_default_simple(&env->simple, TESTS_APP, env->init->priority);
    config = process_config_mcp(config, seL4_MaxPrio);
    config = process_config_auth(config, simple_get_tcb(&env->simple));
    config = process_config_create_cnode(config, TEST_PROCESS_CSPACE_SIZE_BITS);
    config = process_config_fault_endpoint(config, env->root_task_endpoint);
    error = sel4utils_configure_process_custom(&process->native, &env->vka, &env->vspace, config);
    assert(error == 0);

    /* set up caps about the process */
    env->init->stack_pages = CONFIG_SEL4UTILS_STACK_SIZE / PAGE_SIZE_4K;
    env->init->stack = process->native.thread.stack_top - CONFIG_SEL4UTILS_STACK_SIZE;
    env->init->page_directory = sel4utils_copy_cap_to_process(&process->native, &env->vka, process->native.pd.cptr);
    env->init->root_cnode = SEL4UTILS_CNODE_SLOT;
    env->init->tcb = sel4utils_copy_cap_to_process(&process->native, &env->vka, process->native.thread.tcb.cptr);
    if (config_set(CONFIG_HAVE_TIMER)) {
        env->init->timer_ntfn = sel4utils_copy_cap_to_process(&process->native, &env->vka, env->timer_notify_test.cptr);
    }

    env->init->domain = sel4utils_copy_cap_to_process(&process->native, &env->vka, simple_get_init_cap(&env->simple,
                                                                                                           seL4_CapDomain));
    env->init->asid_pool = sel4utils_copy_cap_to_process(&process->native, &env->vka, simple_get_init_cap(&env->simple,
                                                                                                              seL4_CapInitThreadASIDPool));
    env->init->asid_ctrl = sel4utils_copy_cap_to_process(&process->native, &env->vka, simple_get_init_cap(&env->simple,
                                                                                                              seL4_CapASIDControl));
#ifdef CONFIG_IOMMU
    env->init->io_space = sel4utils_copy_cap_to_process(&process->native, &env->vka, simple_get_init_cap(&env->simple,
                                                                                                             seL4_CapIOSpace));
#endif /* CONFIG_IOMMU */
#ifdef CONFIG_TK1_SMMU
    env->init->io_space_caps = arch_copy_iospace_caps_to_process(&process->native, &env);
#endif
    env->init->cores = simple_get_core_count(&env->simple);
    /* copy the sched ctrl caps to the remote process */
    if (config_set(CONFIG_KERNEL_MCS)) {
        seL4_CPtr sched_ctrl = simple_get_sched_ctrl(&env->simple, 0);
        env->init->sched_ctrl = sel4utils_copy_cap_to_process(&process->native, &env->vka, sched_ctrl);
        for (int i = 1; i < env->init->cores; i++) {
            sched_ctrl = simple_get_sched_ctrl(&env->simple, i);
            sel4utils_copy_cap_to_process(&process->native, &env->vka, sched_ctrl);
        }
    }
    /* setup data about untypeds */
    printf("Got %i Total untypeds for processes\n", env->num_untypeds);
    int num_untyped_per_process = 8; // env->num_untypeds
    env->init->untypeds = copy_untypeds_to_process(&process->native, env->untypeds, num_untyped_per_process, env);
    /* copy the fault endpoint - we wait on the endpoint for a message
     * or a fault to see when the test finishes */
    //env->endpoint = sel4utils_copy_cap_to_process(&(env->test_process), &env->vka, env->test_process.fault_endpoint.cptr);

    // create a minted enpoint for the process
    cspacepath_t path;
    vka_cspace_make_path(&env->vka, env->root_task_endpoint.cptr/* test_process.fault_endpoint.cptr*/, &path);
    env->process_endpoint = sel4utils_mint_cap_to_process(&process->native, path, seL4_AllRights, 1234 );


    /* copy the device frame, if any */
    if (env->init->device_frame_cap) {
        env->init->device_frame_cap = sel4utils_copy_cap_to_process(&process->native, &env->vka, env->device_obj.cptr);
    }

    /* map the cap into remote vspace */
    env->remote_vaddr = vspace_share_mem(&env->vspace, &process->native.vspace, env->init, 1, PAGE_BITS_4K,
                                         seL4_AllRights, 1);
    assert(env->remote_vaddr != 0);

    /* WARNING: DO NOT COPY MORE CAPS TO THE PROCESS BEYOND THIS POINT,
     * AS THE SLOTS WILL BE CONSIDERED FREE AND OVERRIDDEN BY THE TEST PROCESS. */
    /* set up free slot range */
    env->init->cspace_size_bits = TEST_PROCESS_CSPACE_SIZE_BITS;
    if (env->init->device_frame_cap) {
        env->init->free_slots.start = env->init->device_frame_cap + 1;
    } else {
        env->init->free_slots.start = env->process_endpoint + 1;
    }
    env->init->free_slots.end = (1u << TEST_PROCESS_CSPACE_SIZE_BITS);
    assert(env->init->free_slots.start < env->init->free_slots.end);
}


void basic_run_test(const char *name, driver_env_t env, Process* process)
{
    int error;

    /* copy test name */
    strncpy(env->init->name, name, TEST_NAME_MAX);
    /* ensure string is null terminated */
    env->init->name[TEST_NAME_MAX - 1] = '\0';
#ifdef CONFIG_DEBUG_BUILD
    seL4_DebugNameThread(process->native.thread.tcb.cptr, env->init->name);
#endif

    /* set up args for the test process */
    seL4_Word argc = 2;
    char string_args[argc][WORD_STRING_SIZE];
    char *argv[argc];

    sel4utils_create_word_args(string_args, argv, argc, env->process_endpoint, env->remote_vaddr);

    /* spawn the process */
    error = sel4utils_spawn_process_v(&process->native, &env->vka, &env->vspace,
                                      argc, argv, 1);
    ZF_LOGF_IF(error != 0, "Failed to start test process!");

    if (config_set(CONFIG_HAVE_TIMER)) {
        error = tm_alloc_id_at(&env->tm, TIMER_ID);
        ZF_LOGF_IF(error != 0, "Failed to alloc time id %d", TIMER_ID);
    }
}

void basic_tear_down(driver_env_t env, Process* process)
{
    /* unmap the env->init data frame */
    vspace_unmap_pages(&process->native.vspace, env->remote_vaddr, 1, PAGE_BITS_4K, NULL);

    /* reset all the untypeds for the next test */
    for (int i = 0; i < env->num_untypeds; i++) {
        cspacepath_t path;
        vka_cspace_make_path(&env->vka, env->untypeds[i].cptr, &path);
        vka_cnode_revoke(&path);
    }

    /* destroy the process */
    sel4utils_destroy_process(&process->native, &env->vka);
}

