#pragma once


#include "Bootstrap.h"
#include "IODevice.h"

#include <sel4platsupport/timer.h>


typedef struct
{
    IOBaseDevice super;
    seL4_timer_t   timer;

} MasterTimerDriver;


MasterTimerDriver* TimerDriverInit(InitContext* context, seL4_CPtr notifCap);

