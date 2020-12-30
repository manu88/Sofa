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
#pragma once
#include <helpers.h>
#include <sel4utils/thread.h>

typedef struct _Thread Thread;

typedef int (*ThreadMain)(Thread* thread, void *arg);

typedef struct _Thread
{
    sel4utils_thread_t th;
    ThreadMain main;
    seL4_CPtr ep;

    void* sofaIPC;
    
} Thread;


int ThreadInit(Thread* t, ThreadMain threadMain, void* arg);

int ThreadJoin(Thread* t, void **retval);