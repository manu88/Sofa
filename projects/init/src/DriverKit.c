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
#include <data_struct/chash.h>

#define MAX_SIZE_HASH_DRIVERS 20

typedef struct
{
    InitContext* context;
    chash_t _devices;
    
    
} DriverKitContext;

static DriverKitContext _DKContext;

int DriverKitInit(InitContext* context)
{	
//	seL4_CPtr cap = simple_get_IOPort_cap(&context->simple, 1,1);
// cspace_irq_control_get_cap( simple_get_cnode(&context->simple)) , seL4_CapIRQControl, 1);
    memset(&_DKContext , 0 , sizeof(DriverKitContext));
    _DKContext.context = context;
    
    chash_init(&_DKContext._devices, MAX_SIZE_HASH_DRIVERS);
    
    if(_DKContext._devices.table == NULL)
    {
        return 0;
    }
    
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
    

    if( chash_set(&_DKContext._devices, badge, device) == 0)
    {
        device->_badge = badge;
	return 1;
    }
    return 0;

}

int DriverKitRemoveDevice( IOBaseDevice* device)
{

    return 0;
}

IOBaseDevice* DriverKitGetDeviceForBadge( seL4_Word badge)
{
    return chash_get(&_DKContext._devices, (uint32_t) badge);
}
