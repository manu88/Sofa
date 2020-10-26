#pragma once

#include <simple/simple.h>
#include <vka/vka.h>
#include <vka/object.h>
#include <vspace/vspace.h>
#include <sel4platsupport/io.h>
#include <platsupport/ltimer.h>
#include <platsupport/time_manager.h>

#define MAX_TIMER_IRQS 4

typedef struct
{
    /* An initialised vka that may be used by the test. */
    vka_t vka;
    /* virtual memory management interface */
    vspace_t vspace;
    /* abtracts over kernel version and boot environment */
    simple_t simple;

    /* IO ops for devices */
    ps_io_ops_t ops;

    /* logical timer interface */
    ltimer_t ltimer;

    vka_object_t rootTaskEP;

    /* The main timer notification that sel4-driver receives ltimer IRQ on */
    vka_object_t timer_notification;


    /* time server for managing timeouts */
    time_manager_t tm;

    int num_timer_irqs;

    /* timer IRQ handler caps */
    sel4ps_irq_t timer_irqs[MAX_TIMER_IRQS];

    /* The badged notifications that are paired with the timer IRQ handlers */
    cspacepath_t badged_timer_notifications[MAX_TIMER_IRQS];


    int num_untypeds;
    vka_object_t *untypeds;
} Environ;


void Environ_init(Environ* env);

unsigned int Environ_populate_untypeds(Environ* env, vka_object_t *untypeds,  uint8_t* untyped_size_bits_list);