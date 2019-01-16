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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DeviceTree.h"
#include <AMLDecompiler.h>


typedef struct
{
    IODevice *rootDevRef;
    
    IODevice *currentDev;
    
    char currentName[5];
    
} DeviceTreeContext;


static int StartScope(AMLDecompiler* decomp,const ParserContext* context, const char* location)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    //printf("Start Scope '%s'\n" , location);
    //strncpy(deviceContext->currentScope, location, 512);
    
    return 0;
}

static int EndScope(AMLDecompiler* decomp,const ParserContext* context, const char* location)
{
    //printf("End Scope '%s'\n" , location);
    return 0;
}

static int OnDefinitionBlock(AMLDecompiler* decomp,const ParserContext* context, const ACPIDefinitionBlock* block)
{

    return 0;
}

static int StartDevice(AMLDecompiler*decomp ,const ParserContext* context, const ACPIDevice* dev)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    
    
    //printf("Start Device '%s' '%s' \n" ,context->currentScope+1, dev->name );
    
    IODevice* parent = DeviceTreeGetDeviceWithPath( deviceContext->rootDevRef, context->currentScope+1);
    
    if (!parent)
    {
        //printf("device NOT found for path '%s'\n" , context->currentScope+1);
        
        parent = malloc(sizeof(IODevice));
        assert(parent);
        assert(IODeviceInit(parent, context->currentScope + 1/* +1 removes starting '.'*/) == OSError_None);
        
        IODeviceAddChild(deviceContext->rootDevRef , parent);
        
    }
    
    
    assert(parent);
    IODevice* iodev = malloc(sizeof(IODevice));
    assert(iodev);
    assert(IODeviceInit(iodev, dev->name) == OSError_None);
    
    IODeviceAddChild(parent , iodev);
    
    
    deviceContext->currentDev = iodev;
    
    
    
    return 0;
}

static int EndDevice(AMLDecompiler*decomp ,const ParserContext* context, const ACPIDevice* dev)
{
    //printf("End Device '%.4s'\n" , dev->id);
    return 0;
}

static int StartName(AMLDecompiler* decomp ,const ParserContext* context, const char* name)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    
    assert(deviceContext->currentDev);
    
    if (strlen(deviceContext->currentName) != 0)
    {
        printf("Warning : '%s' value was NOT parsed\n" ,deviceContext->currentName);
    }
    //assert(strlen(deviceContext->currentName) == 0);
    
    strcpy(deviceContext->currentName, name);
    
    //printf("Got name '%.4s' next op %i\n" , name , context->nextOp);
    return 0;
}

static int OnValue(AMLDecompiler* decomp,const ParserContext* context, uint64_t value)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    
    
    
    if (strcmp("_HID", deviceContext->currentName) == 0)
    {
        assert(deviceContext->currentDev);
        deviceContext->currentDev->hid = value;
    }
    else if (strcmp("_ADR", deviceContext->currentName) == 0)
    {
        assert(deviceContext->currentDev);
        deviceContext->currentDev->adr = value;
    }
    else if (strcmp("_UID", deviceContext->currentName) == 0)
    {
        assert(deviceContext->currentDev);
        deviceContext->currentDev->uid = value;
    }
    
        
    
    deviceContext->currentName[0] = 0;
    return 0;
}

OSError DeviceTreeContructDeviceTree(IODevice* root, const uint8_t* fromDatas, size_t bufferSize)
{
    AMLDecompiler decomp;
    DeviceTreeContext  ctx;
    
    if (AMLDecompilerInit(&decomp) == 0)
    {
        return OSError_InitError;
    }
    
    decomp.callbacks.StartDevice       = StartDevice;
    decomp.callbacks.EndDevice         = EndDevice;
    decomp.callbacks.OnDefinitionBlock = OnDefinitionBlock;
    decomp.callbacks.OnValue           = OnValue;
    decomp.callbacks.StartName         = StartName;
    decomp.callbacks.StartScope        = StartScope;
    decomp.callbacks.EndScope          = EndScope;
    
    
    decomp.userData = &ctx;

    
    ctx.rootDevRef = root;
    ctx.currentDev = NULL;
    
    ctx.currentName[0] = 0;
    AMLParserError err = AMLDecompilerStart(&decomp, fromDatas, bufferSize);
    
    assert( err == AMLParserError_None);
    
    return OSError_None;
}

IODevice* DeviceTreeGetDeviceWithPath(IODevice* root, const char* path)
{
    if (strlen(path) == 0)
        return NULL;
    
    char p[512];
    strncpy(p, path, 512);
    
    char *seg = NULL;
    seg = strtok(p, ".");
    
    IODevice* currentRoot = root;
    
    while (seg != NULL)
    {
        //struct kobject *child = NULL;
        
        if (kset_count( ((struct kset*)currentRoot) ) == 0)
        {
            return NULL;
        }
        
        currentRoot = (IODevice*) kset_getChildByName( (struct kset*) currentRoot, seg);
        /*
        kset_foreach( ((struct kset*)currentRoot), child)
        {
            if ( strcmp( child->k_name  , seg) == 0)
            {
                currentRoot = (IODevice*) child;
                break;
            }
        }
         */
                
        seg = strtok(NULL, ".");
    }
    
    
    return (IODevice*) currentRoot;
}
