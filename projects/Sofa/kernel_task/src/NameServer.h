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
#include "Thread.h"
#include <utils/uthash.h>
#include <sel4/sel4.h>
#include "utlist.h"


typedef struct _Process Process;

typedef struct _Service
{
    char* name;
    Process* owner;

    UT_hash_handle hh;

    seL4_CPtr endpoint;     // endpoint in the context of the process
    seL4_CPtr baseEndpoint; // endpoint in kernel_task side


    seL4_CPtr kernTaskEp; // endpoint employed by kernel_task to send messages to the service
} Service;


typedef struct _ServiceClient
{
    Service* service; // the service it belongs to.
    ThreadBase* caller;
    UT_hash_handle hh; /* makes this structure hashable */
    char* buff; // the IPC buffer


    struct _ServiceClient* next; // For the per thread client list. See Process.Thread.clients

}ServiceClient;

typedef enum
{
    ServiceNotification_ClientExit,
    ServiceNotification_WillStop,
}ServiceNotification;

Service* NameServerGetServices(void);

#define FOR_EACH_SERVICE(service, tmp) HASH_ITER(hh, NameServerGetServices(), service, tmp)

/*
Init a service with a given process owner
*/
static inline void ServiceInit(Service* s, Process* owner)
{
    memset(s, 0, sizeof(Service));
    s->owner = owner;
}

/*
Helper to create a Service inside kernel_task
*/
int ServiceCreateKernelTask(Service* s);


int NameServerInit(void);


int NameServerRegister(Service* service);
int NameServerUnregister(Service* s);


Service* NameServerFind(const char *name);