/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include "Timer.h"
#include <sel4platsupport/io.h>
#include <stdio.h>
#include <errno.h>



int TimerDriverInit(KernelTaskContext* context , seL4_CPtr notifCap)
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

    return error == 0;
}


uint64_t TimerGetTime( KernelTaskContext* context )
{
	uint64_t endTimeNS;
        ltimer_get_time(&context->timer.ltimer, &endTimeNS);

	return endTimeNS;
}
