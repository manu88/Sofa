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

#include "DriverKit.h"
#include "DeviceTree.h"
#include <stdio.h>

typedef struct
{
    struct kset obj;
    IODevice rootDev;
    
} DriverKit;

static char dkObjectName[] = "DriverKit";

static struct kobj_type _driverKitType =
{
    NULL, // release
};

static DriverKit _dk;

OSError DriverKitInit()
{
    kset_init(&_dk.obj);
    _dk.obj.obj.k_name = dkObjectName;
    _dk.obj.obj.ktype  = &_driverKitType;
    
    
    IODeviceInit(&_dk.rootDev, "root");
    DriverKitRegisterDevice(&_dk.rootDev);
    return OSError_None;
}

struct kobject* DriverKitGetObject()
{
    return &_dk.obj.obj;
}

OSError DriverKitRegisterDevice( struct kobject* obj)
{
    return kset_append(&_dk.obj, obj);
}

OSError DriverKitContructDeviceTree( const uint8_t* fromDatas, size_t bufferSize)
{
    OSError err = DeviceTreeContructDeviceTree( (IODevice*) &_dk.rootDev, fromDatas, bufferSize);
    return  err;
}

static void _printObject(const IODevice* object , int indent)
{
    
    
    for(int i =0;i<indent;i++)
        printf("|\t");
    
    printf("|'%s' \n" , object->base.obj.k_name  );//, child->type == INodeType_Folder? "Folder":"File");
    
    for(int i =0;i<indent+1;i++)
        printf("|\t");
    
    printf("- HID 0x%llx UID 0x%llx ADR 0x%llx\n" ,object->hid, object->uid , object->adr );//, child->type == INodeType_Folder? "Folder":"File");
    
    struct kobject *child = NULL;
    kset_foreach( ((struct kset*)object), child)
    {
        _printObject((IODevice*)child, indent +1);
        /*
         if (child->type == INodeType_Folder)
         {
         _printNode(child , indent + 1);
         }
         */
    }
}


void DriverKitDump()
{
    printf("--- DriverKit Tree ---\n");
    
    _printObject( &_dk.rootDev , 0);
    /*
     struct kobject *child = NULL;
     
     kset_foreach( (&_systemTree.root.obj), child)
     {
     printf(" -'%s'\n" , child->k_name);
     }
     */
    printf("--- DriverKit Tree ---\n");
}






