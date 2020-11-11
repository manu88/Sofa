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
/* this file is shared between sel4test-driver an sel4test-tests */
#pragma once
#define CONFIG_HAVE_TIMER 1
#include <autoconf.h>
#include <sel4/bootinfo.h>

#include <platsupport/time_manager.h>
#include <vka/vka.h>
#include <vka/object.h>
#include <sel4utils/process.h>
#include <simple/simple.h>
#include <vspace/vspace.h>

/* Contains information about the test environment for regular tests, bootstrap tests do
 * not use this environment */
struct env {
    /* An initialised vka that may be used by the test. */
    vka_t vka;
    /* virtual memory management interface */
    vspace_t vspace;
    /* abstract interface over application init */
    simple_t simple;
    /* notification for timer */
    vka_object_t timer_notification;

    /* caps for the current process */
    seL4_CPtr cspace_root;
    seL4_CPtr page_directory;
    seL4_CPtr endpoint;
    seL4_CPtr tcb;
    seL4_CPtr timer_untyped;
    seL4_CPtr asid_pool;
    seL4_CPtr asid_ctrl;
    seL4_CPtr sched_ctrl;
#ifdef CONFIG_IOMMU
    seL4_CPtr io_space;
#endif /* CONFIG_IOMMU */
#ifdef CONFIG_TK1_SMMU
    seL4_SlotRegion io_space_caps;
#endif
    seL4_Word cores;
    seL4_CPtr domain;
    seL4_CPtr device_frame;

    int priority;
    int cspace_size_bits;
    int pid;
};
typedef struct env *env_t;

