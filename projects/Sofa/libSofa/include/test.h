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
    seL4_CPtr vspace_root;

    vka_object_t timer_notification;


    uint8_t *mainIPCBuffer;

    int priority;
    int cspace_size_bits;
    int pid;
};
typedef struct env *env_t;

