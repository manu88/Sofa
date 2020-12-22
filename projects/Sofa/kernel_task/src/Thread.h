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
#include <sys/types.h>
#include <sel4/types.h>

typedef struct _ServiceClient ServiceClient;

typedef struct _Process Process;
/*
Shared structure beetwen kernel_task threads and process threads.
See KThread and Thread.
*/
typedef struct
{
    uint8_t kernTaskThread;
    Process* process; //process owner

    seL4_Word replyCap;
    seL4_Word currentSyscallID;

    unsigned int timerID;
    ServiceClient* _clients; // a list of Service clients belonging to this thread.


} ThreadBase;


void ThreadBaseAddServiceClient(ThreadBase*t, ServiceClient* client);