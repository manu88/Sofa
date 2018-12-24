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

#include "Thread.h"
#include <assert.h>
#include <stdio.h>


static void _ThreadStart(void *arg0, void *arg1, void *ipc_buf)
{
	Thread* self = (Thread*) arg0;
	assert(self);


	self->entryPoint(self , arg1 , ipc_buf);
}

int ThreadInit(Thread* thread, vka_t *vka, vspace_t *parent, sel4utils_thread_config_t fromConfig)
{
	return sel4utils_configure_thread_config(vka , parent , /*alloc*/parent , fromConfig , &thread->thread) == 0;
}


int ThreadStart(Thread* thread , void* arg,   int resume)
{
	return sel4utils_start_thread(&thread->thread , _ThreadStart , thread , arg , resume);
}
