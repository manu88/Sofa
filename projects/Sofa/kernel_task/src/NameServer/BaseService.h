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
#include "KThread.h"
#include "NameServer.h"
#include "Thread.h"


typedef struct _BaseService BaseService;
typedef void (*OnServiceStart)(BaseService* service);
typedef void (*OnSystemMsg)(BaseService* service, seL4_MessageInfo_t msg);
typedef void (*OnClientMsg)(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);

typedef struct
{
    OnServiceStart onServiceStart;
    OnSystemMsg onSystemMsg;
    OnClientMsg onClientMsg;
}BaseServiceCallbacks;

typedef struct _BaseService
{
    KThread thread;
    Service service;

    ServiceClient* _clients; // the list of currently connected clients

    BaseServiceCallbacks* callbacks;

}BaseService;


int BaseServiceCreate(BaseService*s, const char*name, BaseServiceCallbacks* ops);

int BaseServiceStart(BaseService*s);


int BaseServiceCreateClientContext(BaseService* service, ThreadBase* caller, ServiceClient* client, size_t numPages);

ServiceClient* BaseServiceGetClient(BaseService*s, ThreadBase* caller);