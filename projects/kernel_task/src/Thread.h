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

#pragma once


#include <sel4utils/thread.h>
#include <sel4utils/thread_config.h>
#include "Bootstrap.h"

struct _Thread;

typedef void (*ThreadEntryPoint)(struct _Thread *self, void *arg, void *ipc_buf);


typedef struct _Thread
{
    sel4utils_thread_t thread;
    ThreadEntryPoint entryPoint;
} Thread;




int ThreadInit(Thread* thread , vka_t *vka, vspace_t *parent, sel4utils_thread_config_t fromConfig);


static inline int ThreadSetPriority(Thread* thread , uint8_t priority)
{
	return seL4_TCB_SetPriority(thread->thread.tcb.cptr, seL4_CapInitThreadTCB ,  priority);
}


int ThreadStart(Thread* thread , void* arg,  int resume);
