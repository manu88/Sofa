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
#include "IODevice.h"


OSError IODeviceInit(IODevice* device, const char* name)
{
    memset(device, 0, sizeof(IODevice));
    kset_init(&device->base);
    
    device->base.obj.k_name =  strdup(name);
    //strncpy(device->name, name, 32);
    return OSError_None;
}

OSError IODeviceAddChild( IODevice* baseDev, IODevice* child)
{
    if (baseDev == child)
    {
        return OSError_ArgError;
    }
    
    return kset_append(&baseDev->base, &child->base.obj);
}
