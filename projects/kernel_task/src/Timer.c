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
#include "Bootstrap.h"

#include <platsupport/time_manager.h>
#include <sel4platsupport/timer.h>
#include <sel4platsupport/io.h>

#define MAX_TIMERS 64

static time_manager_t _tm = {0};
static seL4_timer_t   timer;


time_manager_t *getTM()
{
    return &_tm;
}
int TimerInit(ps_io_ops_t *ops , seL4_CPtr notifCap)
{
	int err = 0;
    
    err = sel4platsupport_new_io_ops(*getVspace(), *getVka(), ops);
    assert(err == 0);


    err = sel4platsupport_new_arch_ops( ops, getSimple(), getVka());
    assert(err == 0);

    err = sel4platsupport_init_default_timer_ops( getVka(), getVspace(), getSimple(), *ops, notifCap, &timer);

    assert(err == 0);

	err = tm_init(&_tm ,&timer.ltimer ,ops , MAX_TIMERS);
    assert(err == 0);
	

	return err;
}

int TimerProcess(seL4_Word sender_badge)
{
    
    sel4platsupport_handle_timer_irq(&timer, sender_badge);
    return tm_update(&_tm);
}


int TimerAllocAndRegisterOneShot(time_manager_t *tm , uint64_t rel_ns, uint32_t id,  timeout_cb_fn_t callback, uintptr_t token)
{
	int err = tm_alloc_id_at(tm , id);
	if (!err)
	{
		return tm_register_rel_cb( tm , rel_ns , id , callback , token);
	}
	return err;
}


int TimerAllocAndRegister(time_manager_t *tm , uint64_t period_ns, uint64_t start, uint32_t id, timeout_cb_fn_t callback, uintptr_t token)
{
        int err = tm_alloc_id_at(tm , id);
        if( !err)
        {
                return tm_register_periodic_cb(tm , period_ns ,start,id , callback, token);
        }
        return err;
} 

int TimerCancelID( uint32_t id)
{
    return tm_deregister_cb(&_tm  , id);
}
 
uint64_t GetCurrentTime()
{
	uint64_t t;
	
	tm_get_time(&_tm , &t);
	return t;

}
