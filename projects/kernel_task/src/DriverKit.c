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
#include "DriverKit.h"
#include <assert.h>


/* HASH macros for seL4_Word key*/

#define HASH_ADD_SEL4_WORD(head,key,add)  HASH_ADD(hh,head,key,sizeof(seL4_Word),add)
#define HASH_FIND_SEL4_WORD(head,key,out) HASH_FIND(hh,head,key,sizeof(seL4_Word),out)

/* *** **** */

typedef struct
{
    KernelTaskContext* context;
    //chash_t _devices;
    
    IOBaseDevice* _devices;
    
    
} DriverKitContext;

static DriverKitContext _DKContext;

int DriverKitInit(KernelTaskContext* context)
{	
//	seL4_CPtr cap = simple_get_IOPort_cap(&context->simple, 1,1);
// cspace_irq_control_get_cap( simple_get_cnode(&context->simple)) , seL4_CapIRQControl, 1);
    memset(&_DKContext , 0 , sizeof(DriverKitContext));
    _DKContext.context = context;

    assert(_DKContext._devices == NULL);
	return 1;
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
