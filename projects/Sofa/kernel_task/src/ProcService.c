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
#include "ProcService.h"
#include "BaseService.h"
#include "Process.h"
#include "Log.h"
#include <proc.h>

static BaseService _service;

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg);
static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);


static BaseServiceCallbacks _servOps = {.onSystemMsg=_OnSystemMsg, .onClientMsg=_OnClientMsg};


int ProcServiceInit()
{
    return BaseServiceCreate(&_service, procServiceName, &_servOps);
}

int ProcServiceStart()
{
    return BaseServiceStart(&_service);
}

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("ProcService.on system msg\n");
}


static void onRegister(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("Proc request register\n");
    ServiceClient* client = malloc(sizeof(ServiceClient));
    assert(client);
    int err = BaseServiceCreateClientContext(service, sender, client, 1);
    if(err != 0)
    {
        free(client);
    }
    seL4_SetMR(1, err == 0? client->buffClientAddr:-1);
    seL4_Reply(msg);
}


static void onProcEnum(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{

    ServiceClient* client = BaseServiceGetClient(service, sender);
    assert(client);

    size_t procCount = ProcessListCount();
    //KLOG_DEBUG("Proc enum request: %zi processes\n", procCount);    

    Process*p = NULL;

    size_t accSize = 0;
    char* buff = client->buff; 
    FOR_EACH_PROCESS(p)
    {

        ProcessDesc* desc = (ProcessDesc*) buff;
        desc->startTime = p->stats.startTime;
        desc->pid = ProcessGetPID(p);
        desc->nameLen = strlen(ProcessGetName(p));
        strncpy(desc->name, ProcessGetName(p), desc->nameLen);
        size_t recSize = sizeof(ProcessDesc) + desc->nameLen;

        accSize += recSize; 

        buff += recSize;
    }
    seL4_SetMR(1, procCount);
    seL4_Reply(msg);
}

static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    ProcRequest req = (ProcRequest) seL4_GetMR(0);

    switch (req)
    {
    case ProcRequest_Register:
        onRegister(service, sender, msg);
        break;
    case ProcRequest_Enum:
        onProcEnum(service, sender, msg);
        break;
    default:
        break;
    } 
}