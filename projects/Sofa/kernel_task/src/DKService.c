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
#include "devFS.h"
#include <dk.h>

BaseService _dkService;


static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg);
static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);


static BaseServiceCallbacks _dkOps = {.onServiceStart=NULL, .onSystemMsg=_OnSystemMsg, .onClientMsg=_OnClientMsg};

int DKServiceInit()
{
    return BaseServiceCreate(&_dkService, "deviceKit", &_dkOps);
}


static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("DKService: msg from kernel_task!\n");
}

static int DKEnumRequest(ServiceClient* clt, IODeviceType type, int onlyCount)
{
    IODevice* dev = NULL;
    int count = 0;
    assert(clt->buff);
    DKDeviceList* outList = (DKDeviceList*) clt->buff;
    FOR_EACH_DEVICE(dev)
    {
        if(type == IODevice_AllTypes || dev->type == type)
        {   
            if(!onlyCount)
            {
                outList->handles[count] = dev;    
            }
            count++;
        }
    }
    if(!onlyCount)
    {
        outList->count = count;
    }

    return count;
}

static long doDKDeviceDetails(BaseService* service, ThreadBase* caller, seL4_MessageInfo_t msg)
{
    ServiceClient* clt = BaseServiceGetClient(service, caller);
    assert(clt);
    const IODevice* dev = (const IODevice*) seL4_GetMR(1);
    DKDeviceDetails code = (DKDeviceDetails) seL4_GetMR(2);

    switch (code)
    {
        case DKDeviceDetail_GetName:
            strcpy(clt->buff, dev->name);
            return strlen(dev->name);

        case DKDeviceDetail_GetDevFile:
        {
            DevFile* f = DevFSGetFileForDevice(dev);
            if(f)
            {
                strcpy(clt->buff, f->name);
                return strlen(f->name);
            }
            return 0;
        }
        default:
            KLOG_DEBUG("doDKDeviceDetails Unknown DKDeviceDetails code %i on %s\n", code, dev->name);
            return -EINVAL;
            break;
    }
}

static int doDKEnumRequest(BaseService* service, ThreadBase* caller, seL4_MessageInfo_t msg)
{

    int type = seL4_GetMR(1);
    int onlyCount = seL4_GetMR(2);
    int ret = 0;
    if(type >= IODevice_Last)
    {
        ret = -EINVAL;
    }
    else
    {
        ServiceClient* clt = BaseServiceGetClient(service, caller);
        assert(clt);
        ret = DKEnumRequest(clt, type, onlyCount);
    }
    
    seL4_SetMR(1, ret);
    seL4_Reply(msg);
}

static void onRegister(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
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

static void _OnClientMsg(BaseService* service, ThreadBase* caller, seL4_MessageInfo_t msg)
{
    DKRequest req = (DKRequest) seL4_GetMR(0);

    switch (req)
    {
        case DKRequest_Register:
        {
            onRegister(service, caller, msg);
            break;
        }
        case DKRequest_List:
        {
            KLOG_DEBUG("DKService: Dev Enum request\n");
            IODevice* dev = NULL;
            FOR_EACH_DEVICE(dev)
            {
                KLOG_INFO("'%s' type %i\n", dev->name, dev->type);
            }

            break;
        }
        case DKRequest_Tree:
        {
            KLOG_DEBUG("DKService: IONode tree request\n");
            DeviceTreePrint(DeviceTreeGetRoot());

            break;
        }
        case DKRequest_DeviceDetails:
        {
            long ret = doDKDeviceDetails(service, caller, msg);
            seL4_SetMR(2, ret);
            seL4_Reply(msg);
            break;
        }
        case DKRequest_Enum:
        {
            doDKEnumRequest(service, caller, msg);
            break;
        }
    }
}

int DKServiceStart()
{
    return BaseServiceStart(&_dkService);
}