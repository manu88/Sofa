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
#include <allocman/bootstrap.h>
#include <simple-default/simple-default.h>
#include <simple-default/simple-default.h>
#include <sel4utils/vspace.h>

#include <sel4platsupport/arch/io.h>

/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE (BIT(seL4_PageBits) * 10)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE (BIT(seL4_PageBits) * 100)


/* static memory for virtual memory bootstrapping */
UNUSED static sel4utils_alloc_data_t data;


int bootstrapSystem(KernelTaskContext *context)
{
    UNUSED int error = 0;

    simple_default_init_bootinfo(&context->simple, context->info);

    /* create an allocator */
    context->allocman = bootstrap_use_current_simple(&context->simple, ALLOCATOR_STATIC_POOL_SIZE,
                                            allocator_mem_pool);

    ZF_LOGF_IF(context->allocman == NULL, "Failed to initialize allocator.\n"
               "\tMemory pool sufficiently sized?\n"
               "\tMemory pool pointer valid?\n");

    /* create a vka (interface for interacting with the underlying allocator) */
    allocman_make_vka(&context->vka, context->allocman);


    error = sel4utils_bootstrap_vspace_with_bootinfo_leaky( &context->vspace , &data ,simple_get_pd(&context->simple)  ,&context->vka, context->info); 
    ZF_LOGF_IFERR(error, "Failed to prepare root thread's VSpace for use.\n"
                  "\tsel4utils_bootstrap_vspace_with_bootinfo reserves important vaddresses.\n"
                  "\tIts failure means we can't safely use our vaddrspace.\n");

    /* fill the allocator with virtual memory */
    void *vaddr;
    UNUSED reservation_t virtual_reservation;
    virtual_reservation = vspace_reserve_range(&context->vspace,
                                               ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights, 1, &vaddr);
    ZF_LOGF_IF(virtual_reservation.res == NULL, "Failed to reserve a chunk of memory.\n");
    bootstrap_configure_virtual_pool(context->allocman, vaddr,
                                     ALLOCATOR_VIRTUAL_POOL_SIZE, simple_get_pd(&context->simple));


    sel4platsupport_get_io_port_ops(&context->opsIO.io_port_ops, &context->simple , &context->vka);


    return error;
}



