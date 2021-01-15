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
#include "runtime.h"
#include <Sofa.h>
#include <stdarg.h>
#include "dk.h"


seL4_CPtr dkCap = 0;
char* dkBuf = NULL;


int DKClientInit()
{
    if(dkCap && dkBuf)
    {
        return 0;
    }

    ssize_t capOrErr = SFGetService(DeviceKitServiceName);

    if(capOrErr > 0)
    {
        dkCap = capOrErr;

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, DKRequest_Register);
        seL4_Call(dkCap, info);
        dkBuf = (char*) seL4_GetMR(1);
        return 0;
    }
    return -1;
}

int DKClientTempListDevices(void)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, DKRequest_List);
    seL4_Send(dkCap, info);
    return 0;
}

int DKClientEnumDevices(int type, DKDeviceList* list, size_t* numDevices)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, DKRequest_Enum);
    seL4_SetMR(1, type);
    seL4_SetMR(2, list? 0:1); // onlyCount flag
    seL4_Call(dkCap, info);
    seL4_Word ret = seL4_GetMR(1);

    if(list)
    {
        const DKDeviceList* listSrc = (const DKDeviceList*) dkBuf; 
        size_t bufSize = sizeof(DKDeviceList) + (listSrc->count*sizeof(DKDeviceHandle));
        memcpy(list, dkBuf, bufSize);
    }
    if((int)ret < 0)
    {
        return ret;
    }
    
    *numDevices = ret;
    
    return 0;
}

static seL4_Word doDeviceDetailsCall(DKDeviceHandle devHandle, DKDeviceDetails code)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, DKRequest_DeviceDetails);
    seL4_SetMR(1, devHandle);
    seL4_SetMR(2, code);
    seL4_Call(dkCap, info);
    return seL4_GetMR(2);
}

/* Per device operations*/
char* DKDeviceGetName(DKDeviceHandle devHandle)
{
    long ret = (long) doDeviceDetailsCall(devHandle, DKDeviceDetail_GetName);
    if(ret > 0)
    {
        return strdup(dkBuf);
    }
    return NULL;
}
char* DKDeviceGetDevFile(DKDeviceHandle devHandle)
{
    long ret = (long) doDeviceDetailsCall(devHandle, DKDeviceDetail_GetDevFile);
    if(ret > 0)
    {
        return strdup(dkBuf);
    }
    return NULL;
}

int DKClientEnumTree()
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, DKRequest_Tree);
    seL4_Send(dkCap, info);
    return 0;
}