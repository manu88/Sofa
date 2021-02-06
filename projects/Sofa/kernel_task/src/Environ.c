/*
 * This file is part of the Sofa project
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
#include "Environ.h"
#include <allocman/bootstrap.h>
#include <sel4utils/vspace.h>
#include <sel4platsupport/bootinfo.h>
#include <simple-default/simple-default.h>
#include <simple/simple_helpers.h>
#include <allocman/vka.h>
#include <sel4platsupport/io.h>

#include <muslcsys/vsyscall.h>
#include "ProcessList.h"
#include "KThread.h"

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 400)


/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 100)
static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* static memory for virtual memory bootstrapping */
static sel4utils_alloc_data_t data;

static KernelTaskContext _ctx;
uint8_t _env_set = 0;

KernelTaskContext* getKernelTaskContext(void)
{
    return &_ctx;
}

static KMutex _mainVSpaceLock;
static Process _kernelTask = {0};

Process* getKernelTaskProcess()
{
    return &_kernelTask;
}


vspace_t* getMainVSpace()
{
    return &_ctx._vspace;
}

int MainVSpaceLock()
{
    return KMutexLock(&_mainVSpaceLock);
}

int MainVSpaceUnlock()
{
    return KMutexUnlock(&_mainVSpaceLock);
}

vka_t* getMainVKA()
{
    return &_ctx._vka;
}

int IOInit()
{
    KernelTaskContext* env = getKernelTaskContext();


    int error = sel4platsupport_get_io_port_ops(&env->ops.io_port_ops, &env->simple, &env->_vka);
    assert(error == 0);

    vspace_t* mainVSpace = getMainVSpace();
    error = sel4utils_new_page_dma_alloc(&env->_vka, mainVSpace, &env->ops.dma_manager);
    assert(error == 0);
}

/* globals for malloc */
extern vspace_t *muslc_this_vspace;
extern reservation_t muslc_brk_reservation;
extern void *muslc_brk_reservation_start;
static sel4utils_res_t malloc_res;

static void CONSTRUCTOR(MUSLCSYS_WITH_VSYSCALL_PRIORITY)  init_malloc(void)
{
    KernelTaskContext* env = getKernelTaskContext();
    int error;

    seL4_BootInfo *info = platsupport_get_bootinfo(); // seL4_GetBootInfo();
    simple_default_init_bootinfo(&env->simple, info);

    
/* create allocator */
    env->allocman = bootstrap_use_current_simple(&env->simple, ALLOCATOR_STATIC_POOL_SIZE, allocator_mem_pool);
    assert(env->allocman);

    /* create vka */
    allocman_make_vka(&env->_vka, env->allocman);

    /* create vspace */
    error = sel4utils_bootstrap_vspace_with_bootinfo_leaky(&env->_vspace, &data, simple_get_pd(&env->simple),
                                                           &env->_vka, info);

    /* set up malloc */
    error = sel4utils_reserve_range_no_alloc(&env->_vspace, &malloc_res, seL4_LargePageBits, seL4_AllRights, 1,
                                             &muslc_brk_reservation_start);
    muslc_this_vspace = &env->_vspace;
    muslc_brk_reservation.res = &malloc_res;
    ZF_LOGF_IF(error, "Failed to set up dynamic malloc");

    void *vaddr;
    reservation_t virtual_reservation = vspace_reserve_range(&env->_vspace, ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights,
                                               1, &vaddr);
    assert(virtual_reservation.res);
    bootstrap_configure_virtual_pool(env->allocman, vaddr, ALLOCATOR_VIRTUAL_POOL_SIZE, simple_get_pd(&env->simple));

    KMutexNew(&_mainVSpaceLock);
    
    _env_set = 1;

}