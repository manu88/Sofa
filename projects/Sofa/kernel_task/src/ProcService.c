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
    return BaseServiceCreate(&_service, "Proc", &_servOps);
}

int ProcServiceStart()
{
    return BaseServiceStart(&_service);
}


static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("ProcService.on system msg\n");
}

static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("ProcService. client msg from %i\n", ProcessGetPID(sender->process));

    ProcRequest req = (ProcRequest) seL4_GetMR(0);

    switch (req)
    {
    case ProcRequest_Register:
        KLOG_DEBUG("Proc request register\n");
        seL4_Reply(msg);
        break;
    case ProcRequest_Enum:
        KLOG_DEBUG("Proc request enum\n");
        seL4_Reply(msg);
        break;
    default:
        break;
    } 
}