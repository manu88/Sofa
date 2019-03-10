/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include <string.h>
#include <stdio.h>

#include <assert.h>

#include "DriverKit.h"

#ifndef SOFA_TESTS_ONLY
#include <pci/pci.h>
#endif

#include "fs.h"


/* HASH macros for seL4_Word key*/

#define HASH_ADD_SEL4_WORD(head,key,add)  HASH_ADD(hh,head,key,sizeof(seL4_Word),add)
#define HASH_FIND_SEL4_WORD(head,key,out) HASH_FIND(hh,head,key,sizeof(seL4_Word),out)

/* *** **** */

typedef struct
{
    KernelTaskContext* context;
    //chash_t _devices;
    
    IOBaseDevice* _devices;
    
    Inode deviceNode; // 'sys/devices'
    Inode systemNode; // 'sys/devices/system/
} DriverKitContext;

static DriverKitContext _DKContext;


Inode* DriverKitGetDeviceNode()
{
	return &_DKContext.deviceNode;
}

static int _ScanPCIDevices(KernelTaskContext* context)
{
#ifndef SOFA_TESTS_ONLY
/*    if(libpci_num_devices == 0)
    {
        libpci_scan( context->ops.io_port_ops);
    }
    
    return libpci_num_devices;*/
#else
    return 0;
#endif
}

static const char DeviceFolderName[] = "devices";
static const char SystemFolderName[] = "system";

int DriverKitInit(KernelTaskContext* context)
{	
    memset(&_DKContext , 0 , sizeof(DriverKitContext));
    _DKContext.context = context;

    assert(_DKContext._devices == NULL);

    int numPCIDevices = _ScanPCIDevices(context);
    
    printf("Got %i pci devices\n", numPCIDevices);

    int error = InodeInit(&_DKContext.deviceNode , INodeType_Folder ,  DeviceFolderName);

    assert(error == 1);

    error = InodeInit(&_DKContext.systemNode , INodeType_Folder , SystemFolderName);

    assert(error == 1);

    error = InodeAddChild(&_DKContext.deviceNode , &_DKContext.systemNode );

    assert(error == 1);

    return error;
}



int DriverKitRegisterDevice(seL4_Word badge, IOBaseDevice* device)
{
    if (badge == 0)
    {
        return 0;
    }
    if (device->DeInitDevice == NULL || device->InitDevice == NULL)
    {
        return 0;
    }
    if (device->InitDevice(device) == 0)
    {
        return 0;
    }
    device->_badge = badge;
    HASH_ADD_SEL4_WORD(_DKContext._devices, _badge, device);
    
	return 1;
    
}

int DriverKitRemoveDevice( IOBaseDevice* device)
{
    IOBaseDevice* el = NULL;
    IOBaseDevice* tmp = NULL;
    
    HASH_ITER(hh, _DKContext._devices, el, tmp)
    {
        if (el == device)
        {
            HASH_DEL(_DKContext._devices, el);
            return 1;
        }
    }
    return 0;
}

IOBaseDevice* DriverKitGetDeviceForBadge( seL4_Word badge)
{
    IOBaseDevice* dev = NULL;
    
    HASH_FIND_SEL4_WORD(_DKContext._devices, &badge, dev);

    return dev;
}
