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

#include <platsupport/local_time_manager.h>

#define MAX_TIMERS 64


static  time_manager_t* _tm = NULL;

int TimerInit(KernelTaskContext* ctx , seL4_CPtr notifCap)
{
	int err = 0;
/* vka_alloc_notification(&ctx->vka, &timer_aep);
        assert(err == 0);
*/

        err = sel4platsupport_new_io_ops(ctx->vspace, ctx->vka, &ctx->ops);
        assert(err == 0);


        err = sel4platsupport_new_arch_ops(&ctx->ops, &ctx->simple, &ctx->vka);
        assert(err == 0);

/*
        cspacepath_t notification_path;

        vka_cspace_make_path( &ctx->vka, timer_aep->cptr, &notification_path);
*/
        err = sel4platsupport_init_default_timer_ops(&ctx->vka, &ctx->vspace, &ctx->simple, ctx->ops,
                                                   notifCap, &ctx->timer);


        assert(err == 0);

	err = tm_init(&ctx->tm ,&ctx->timer.ltimer ,&ctx->ops , MAX_TIMERS);
    	assert(err == 0); 
	_tm = &ctx->tm;

	return err;
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


uint64_t GetCurrentTime()
{
	uint64_t t;
	
	tm_get_time(_tm , &t);
	return t;
}
