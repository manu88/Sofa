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
#include "DKService.h"
#include "NameServer.h"
#include "Environ.h"
#include "KThread.h"
#include "Log.h"
#include "Process.h"
#include "DeviceKit/DeviceTree.h"
#include "BaseService.h"
#include <dk.h>

BaseService _dkService;

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg);
static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);


static BaseServiceCallbacks _dkOps = {.onSystemMsg=_OnSystemMsg, .onClientMsg=_OnClientMsg};

int DKServiceInit()
{
    return BaseServiceCreate(&_dkService, "deviceKit", &_dkOps);
}


static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("DKService: msg from kernel_task!\n");
}


static int DKEnumRequest(IODeviceType type)
{
    KLOG_DEBUG("DKEnumRequest for type %i\n", type);

    IODevice* dev = NULL;
    int count = 0;
    FOR_EACH_DEVICE(dev)
    {
        if(dev->type == type)
        {   
            KLOG_INFO("'%s'\n", dev->name);
            count++;
        }
    }

    return count;
}
static int doDKEnumRequest(BaseService* service, ThreadBase* caller, seL4_MessageInfo_t msg)
{

    int type = seL4_GetMR(1);
    int ret = 0;
    if(type >= IODevice_Last)
    {
        ret = -EINVAL;
    }
    else
    {
        ret = DKEnumRequest(type);
    }
    
    seL4_SetMR(1, ret);
    seL4_Reply(msg);
}

static void _OnClientMsg(BaseService* service, ThreadBase* caller, seL4_MessageInfo_t msg)
{
    KernelTaskContext* env = getKernelTaskContext();
    if(seL4_GetMR(0) == DKRequest_Register)
    {
        KLOG_DEBUG("DKService: Register request\n");
        char* buff = vspace_new_pages(&env->vspace, seL4_ReadWrite, 1, PAGE_BITS_4K);
        assert(buff);
        void* buffShared = vspace_share_mem(&env->vspace,
                                            &caller->process->native.vspace,
                                            buff,
                                            1,
                                            PAGE_BITS_4K,
                                            seL4_ReadWrite,
                                            1
                                            );
        assert(buffShared);
        ServiceClient* client = malloc(sizeof(ServiceClient));
        assert(client);
        memset(client, 0, sizeof(ServiceClient));
        client->caller = caller;
        client->buff = buff;
        client->service = &_dkService.service;
        //HASH_ADD_PTR(_clients, caller,(ServiceClient*) client);
        ThreadBaseAddServiceClient(caller, client);
        seL4_SetMR(1, (seL4_Word) buffShared);
        seL4_Reply(msg);
    }
    else if(seL4_GetMR(0) == DKRequest_List)
    {
        KLOG_DEBUG("DKService: Dev Enum request\n");
        IODevice* dev = NULL;
        FOR_EACH_DEVICE(dev)
        {
            KLOG_INFO("'%s' type %i\n", dev->name, dev->type);
        }
    }
    else if(seL4_GetMR(0) == DKRequest_Tree)
    {
        KLOG_DEBUG("DKService: IONode tree request\n");
        DeviceTreePrint(DeviceTreeGetRoot());
    }
    else if(seL4_GetMR(0) == DKRequest_Enum)
    {
        doDKEnumRequest(service, caller, msg);
    }
}

int DKServiceStart()
{
    return BaseServiceStart(&_dkService);
}