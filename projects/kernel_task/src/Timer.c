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
    
    printf("[Timer] sel4platsupport_new_io_ops \n");
    err = sel4platsupport_new_io_ops( getVspace(), getVka(), getSimple(),ops);
    assert(err == 0);

    printf("[Timer] sel4platsupport_new_arch_ops \n");
    err = sel4platsupport_new_arch_ops( ops, getSimple(), getVka());
    assert(err == 0);

    printf("[Timer] sel4platsupport_init_default_timer_ops \n");
    err = sel4platsupport_init_default_timer_ops( getVka(), getVspace(), getSimple(), *ops, notifCap, &timer);

    assert(err == 0);

    printf("[Timer] tm_init \n");
	err = tm_init(&_tm ,&timer.ltimer ,ops , MAX_TIMERS);
    assert(err == 0);
	

	return err;
}

int TimerProcess(seL4_Word sender_badge)
{
    
    sel4platsupport_handle_timer_irq(&timer, sender_badge);
    return tm_update(&_tm);
}

int TimerAllocIDAt( unsigned int id)
{
    return tm_alloc_id_at(&_tm , id);
}

int TimerAllocID( unsigned int *id)
{
    return tm_alloc_id(&_tm , id);
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
