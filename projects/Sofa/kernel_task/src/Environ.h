/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
#include <allocman/allocman.h>
#include <platsupport/time_manager.h>
#include <vka/vka.h>
#include <vka/object.h>
#include <sel4platsupport/irq.h>
#include <simple/simple.h>
#include <vspace/vspace.h>
#include <platsupport/chardev.h>

/* This file is shared with seltest-tests. */
#include "test_init_data.h"
#include "utlist.h"


#define MAX_TIMER_IRQS 4

struct timer_callback_info {
    irq_callback_fn_t callback;
    void *callback_data;
};
typedef struct timer_callback_info timer_callback_info_t;


typedef enum
{
    SystemState_Running,
    SystemState_Halting
}SystemState;

struct _KernelTaskContext 
{
    SystemState _sysState;
    allocman_t *allocman;
    /* An initialised vka that may be used by the test. */
    vka_t _vka;
    /* virtual memory management interface */
    vspace_t _vspace;
    /* abtracts over kernel version and boot environment */
    simple_t simple;

    /* IO ops for devices */
    ps_io_ops_t ops;

    ps_io_mapper_t io_mapper;

    /* logical timer interface */
    ltimer_t ltimer;

    /* The main timer notification that sel4-driver receives ltimer IRQ on */
    vka_object_t timer_notification;

    /* The badged notifications that are paired with the timer IRQ handlers */
    cspacepath_t badged_timer_notifications[MAX_TIMER_IRQS];

    /* A notification used by sel4-driver to signal sel4test-tests that there
     * is a timer interrupt. The timer_notify_test is copied to new tests
     * before actually starting them.
     */
    vka_object_t timer_notify_test;

    /* Only needed if we're on RT kernel */
    vka_object_t reply;

    int num_timer_irqs;

    /* timer IRQ handler caps */
    sel4ps_irq_t timer_irqs[MAX_TIMER_IRQS];
    /* timer callback information */
    timer_callback_info_t timer_cbs[MAX_TIMER_IRQS];

    vka_object_t root_task_endpoint;

//    vka_object_t *untypeds;
    /* device frame to use for some tests */
    vka_object_t device_obj;

    /* time server for managing timeouts */
    time_manager_t tm;

    ps_chardevice_t comDev;
    ps_chardevice_t comDev2;    
    cspacepath_t handler;
};

typedef struct _KernelTaskContext KernelTaskContext;

void plat_init(KernelTaskContext *env) WEAK;

#ifdef CONFIG_TK1_SMMU
seL4_SlotRegion arch_copy_iospace_caps_to_process(sel4utils_process_t *process, KernelTaskContext* env);
#endif



int IOInit(void);

vspace_t* getMainVSpace(void);

// lock ops for each use of `getMainVSpace`
int MainVSpaceLock(void);
int MainVSpaceUnlock(void);

vka_t* getMainVKA(void);


// lock ops for each use of `getMainVKA`
int MainVKALock(void);
int MainVKAUnlock(void);

KernelTaskContext* getKernelTaskContext(void);

typedef struct _Process Process;
Process* getKernelTaskProcess(void);

