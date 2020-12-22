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


// FIXME: Merge  helper_thread_t and this struct together
typedef struct
{
    helper_thread_t th; // Needs to remain 1st!
    seL4_CPtr ep;

    void* ret;
} Thread;

typedef void *(*start_routine) (void *);

int ThreadInit(Thread* t, start_routine threadMain, void* arg);

int ThreadJoin(Thread* t, void **retval);