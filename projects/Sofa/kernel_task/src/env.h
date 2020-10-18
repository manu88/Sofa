#pragma once

#include <simple/simple.h>
#include <vka/vka.h>
#include <vka/object.h>
#include <vspace/vspace.h>
#include <sel4platsupport/io.h>
#include <platsupport/ltimer.h>
#include <platsupport/time_manager.h>

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

    /* A notification used by sel4-driver to signal sel4test-tests that there
     * is a timer interrupt. The timer_notify_test is copied to new tests
     * before actually starting them.
     */
    vka_object_t timer_notify_test;


    /* time server for managing timeouts */
    time_manager_t tm;


    int num_untypeds;
    vka_object_t *untypeds;
} Environ;


void Environ_init(Environ* env);

unsigned int Environ_populate_untypeds(Environ* env, vka_object_t *untypeds,  uint8_t* untyped_size_bits_list);