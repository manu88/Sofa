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

#include "DeviceTree_Tests.h"
#include "DeviceTree.h"

void DeviceTree_Tests()
{
    
    IODevice rootDev;
    assert(IODeviceInit(&rootDev, "root") == OSError_None);
    
    IODevice* childRef = DeviceTreeGetDeviceWithPath(&rootDev, "");
    assert( childRef == NULL);
    
    IODevice child;
    assert(IODeviceInit(&child, "child") == OSError_None);
    
    assert( IODeviceAddChild(&rootDev, &child) == OSError_None);
 
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child");
    assert( childRef == &child);
    
    
    IODevice grandSon;
    assert(IODeviceInit(&grandSon, "grandSon") == OSError_None);
    
    assert( IODeviceAddChild(&child, &grandSon) == OSError_None);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child.grandSon");
    assert( childRef == &grandSon);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "");
    assert( childRef == NULL);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child");
    assert( childRef == &child);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child.prout");
    assert( childRef == NULL);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child.grandSon.prout");
    assert( childRef == NULL);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "prout");
    assert( childRef == NULL);
    
    
    IODevice grandSon2;
    assert(IODeviceInit(&grandSon2, "grandSon2") == OSError_None);
    
    assert( IODeviceAddChild(&child, &grandSon2) == OSError_None);
    
    assert(kset_count(&child.base) == 2);
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child.grandSon");
    assert( childRef == &grandSon);
    
    childRef = DeviceTreeGetDeviceWithPath(&rootDev, "child.grandSon2");
    assert( childRef == &grandSon2);
    
    //childRef = DeviceTreeGetDeviceWithPath(&rootDev, ".");
    //assert( childRef == NULL);
    
}
