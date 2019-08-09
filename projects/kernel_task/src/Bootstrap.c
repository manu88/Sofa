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
#include "Config.h"
#include <sel4platsupport/bootinfo.h>
#include <allocman/bootstrap.h>

#include <sel4platsupport/arch/io.h>
#include <simple-default/simple-default.h>
#include <sel4utils/vspace.h>
#include "system.h"
#include "Utils.h"

//#define ALLOCATOR_STATIC_POOL_SIZE (BIT(seL4_PageBits) * 20)
//UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
//#define ALLOCATOR_VIRTUAL_POOL_SIZE (BIT(seL4_PageBits) * 100)


/* static memory for virtual memory bootstrapping */
//UNUSED static sel4utils_alloc_data_t data;

static uint8_t _bootstrap_mem_pool[SEL4OSAPI_BOOTSTRAP_MEM_POOL_SIZE];

struct  _KernelTaskContext
{
    /*
	seL4_BootInfo *info;
	simple_t       simple;
	vka_t          vka;
	allocman_t *   allocman;
	vspace_t       vspace;
    */
    
    struct ps_io_ops    opsIO;
    
    KernelTaskContext _ctx;
    
    
};

static struct  _KernelTaskContext _ctx = {0};

int bootstrapSystem()
{
	int err = 0;
    
    memset(&_ctx , 0 , sizeof(struct  _KernelTaskContext) );
    
    err = sel4osapi_system_initialize(&_bootstrap_mem_pool);
    
    return err;
}

int bootstrapIO()
{
    
    int err = 0;
    sel4platsupport_get_io_port_ops(&_ctx.opsIO.io_port_ops, getSimple() , getVka() );
    
    return err;
}

simple_t* getSimple()
{
    return sel4osapi_system_get_simple();// &_ctx.simple;
}

vka_t* getVka()
{
    return sel4osapi_system_get_vka();// &_ctx.vka;
}

vspace_t* getVspace()
{
    return sel4osapi_system_get_vspace();//&_ctx.vspace;
}

struct ps_io_ops* getIO_OPS()
{
    return &_ctx.opsIO;
}


KernelTaskContext* getKernelTaskContext()
{
    return &_ctx._ctx;
}


sel4osapi_system_t* getSystem()
{
    return sel4osapi_system_get_instanceI();
}
