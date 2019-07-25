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

#include "Bootstrap.h"
#include <sel4platsupport/bootinfo.h>
#include <allocman/bootstrap.h>

#include <sel4platsupport/arch/io.h>
#include <simple-default/simple-default.h>
#include <sel4utils/vspace.h>



#define ALLOCATOR_STATIC_POOL_SIZE (BIT(seL4_PageBits) * 20)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE (BIT(seL4_PageBits) * 100)


/* static memory for virtual memory bootstrapping */
UNUSED static sel4utils_alloc_data_t data;

struct  _KernelTaskContext
{
	seL4_BootInfo *info;
	simple_t       simple;
	vka_t          vka;
	allocman_t *   allocman;
	vspace_t       vspace;
    struct ps_io_ops    opsIO;
    
    KernelTaskContext _ctx;
};


static struct  _KernelTaskContext _ctx = {0};

int bootstapSystem()
{
	int err = 0;
    
    memset(&_ctx , 0 , sizeof(struct  _KernelTaskContext) );

	_ctx.info = platsupport_get_bootinfo();
	simple_default_init_bootinfo(&_ctx.simple, _ctx.info);

	_ctx.allocman = bootstrap_use_current_simple(&_ctx.simple, ALLOCATOR_STATIC_POOL_SIZE,allocator_mem_pool);

	if( _ctx.allocman == NULL)
	{
		return -1;
	}



	/* create a vka (interface for interacting with the underlying allocator) */
	allocman_make_vka(&_ctx.vka, _ctx.allocman);

	err = sel4utils_bootstrap_vspace_with_bootinfo_leaky( &_ctx.vspace , &data ,simple_get_pd(&_ctx.simple)  ,&_ctx.vka, _ctx.info); 

	if( err != 0)
		return err;

	
	void *vaddr;
	UNUSED reservation_t virtual_reservation;
	virtual_reservation = vspace_reserve_range(&_ctx.vspace, ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights, 1, &vaddr);
	

	if( virtual_reservation.res == NULL)
		return -1;
    
    bootstrap_configure_virtual_pool(_ctx.allocman, vaddr,
                                     ALLOCATOR_VIRTUAL_POOL_SIZE, simple_get_pd(&_ctx.simple));
    
    
    sel4platsupport_get_io_port_ops(&_ctx.opsIO.io_port_ops, &_ctx.simple , &_ctx.vka);


	return err;
}

simple_t* getSimple()
{
	return &_ctx.simple;
}

vka_t* getVka()
{
	return &_ctx.vka;
}

vspace_t* getVspace()
{
	return &_ctx.vspace;
}

struct ps_io_ops* getIO_OPS()
{
    return &_ctx.opsIO;
}


KernelTaskContext* getKernelTaskContext()
{
    return &_ctx._ctx;
}
