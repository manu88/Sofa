#pragma once


#include <sel4platsupport/bootinfo.h>
#include <allocman/vka.h>
#include <simple/simple.h>
#include <sel4platsupport/timer.h>
#include <sel4platsupport/io.h>
#include "TimerWheel/TimersWheel.h"



typedef struct
{
    seL4_BootInfo *info;

    cspacepath_t ep_cap_path;

    simple_t       simple;
    vka_t          vka;
    allocman_t *   allocman;
    vspace_t       vspace;

    seL4_timer_t   timer;
    vka_object_t   ntfn_object;
    ps_io_ops_t ops;
    TimersWheel timersWheel;

} InitContext;




int bootstrapSystem(InitContext* context);
