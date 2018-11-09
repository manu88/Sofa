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
//
//  main.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <unistd.h>

#include <stdio.h>
#include "FileServer.h"
#include <assert.h>
#include <errno.h>
#include "DriverKit.h"
#include <data_struct/chash.h>

#include "DevServer.h"

#include "Init_UnitTests.h"

static int cpioCalled = 0;
static Inode* CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
    printf("CpioOpen '%s' flags %i\n" ,pathname , flags);
    
    cpioCalled = 1;
    
    return NULL;
}

static int consoleOpenCalled = 0;
static Inode* ConsoleOpen (struct _DeviceOperations * device, int flags )
{
    consoleOpenCalled = 1;
    Inode* node = malloc(sizeof(Inode) );
    node->operations = &device->fileOps;
    return node;
}


static ssize_t ConsoleWrite (struct _inode *node,  const char*buffer ,size_t size)
{
    printf("ConsolesWrite '%s' %zi \n" , buffer , size);
    
    return (ssize_t)size;
}

int main(int argc, const char * argv[])
{
    doInit_UnitTests();
    
    FileServerHandler cpioHandler;
    assert(FileServerHandlerInit(&cpioHandler  , "cpio" ) );
    cpioHandler.prefix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    assert(DevServerInit() );
    
    assert(FileServerInit() );
    
    assert(FileServerRegisterHandler(&cpioHandler , "/cpio/") );
    assert(InodeGetChildrenCount(FileServerGetRootNode()) == 1);
    assert(cpioHandler.inode._parent == FileServerGetRootNode());
    
    
    assert(FileServerRegisterHandler(getDevServerHandler(), "/dev/" ) );
    assert(InodeGetChildrenCount(FileServerGetRootNode()) == 2);
    assert(getDevServerHandler()->inode._parent == FileServerGetRootNode());
    
    DeviceOperations ops;
    ops.OpenDevice = ConsoleOpen;
    ops.fileOps.Write = ConsoleWrite;
    
    assert(DevServerRegisterFile("/lol", &ops) == 0);
    
    assert(DevServerRegisterFile("console", &ops) );
    

    return 0;
}
