#include "Timer.h"
#include <sel4platsupport/io.h>
#include <stdio.h>
#include <errno.h>


static MasterTimerDriver _timerDriver;

MasterTimerDriver* TimerDriverInit(InitContext* context , seL4_CPtr notifCap)
{
    assert(context);

    int error = 0;

    error = sel4platsupport_new_io_ops(context->vspace, context->vka, &context->ops);
    assert(error == 0);

    error = sel4platsupport_new_arch_ops(&context->ops, &context->simple, &context->vka);
    assert(error == 0);

    error = 0;
    error = sel4platsupport_init_default_timer_ops(&context->vka, &context->vspace, &context->simple, context->ops,
                                                   notifCap, &context->timer);

    if(error != 0)
    {
        printf("sel4platsupport_init_default_timer_ops error %i\n",error);
    }
    
    assert(error == 0);

    if (!IOBaseDeviceInit(&_timerDriver.super) )
    {
	return NULL;
    }
    return &_timerDriver;
}
