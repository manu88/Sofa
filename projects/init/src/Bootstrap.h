#pragma once


#include <sel4platsupport/bootinfo.h>
#include <allocman/vka.h>
#include <simple/simple.h>
#include <sel4platsupport/timer.h>
#include <sel4platsupport/io.h>
#include "TimerWheel/TimersWheel.h"

#include <platsupport/chardev.h>

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



    struct ps_io_ops    opsIO;
//    ps_chardevice_t devKeyboard;

} InitContext;




int bootstrapSystem(InitContext* context);
