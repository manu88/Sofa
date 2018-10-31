//
//  DevServer.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once

#include "FileServer.h"
#include "fs.h"

struct _DeviceOperations
{
    FileOperations fileOps;
    Inode* (*OpenDevice) (struct _DeviceOperations *, int );
    
    void* userContext;
};

typedef  struct _DeviceOperations DeviceOperations;

FileServerHandler* getDevServerHandler(void);

int DevServerInit(void);

int DevServerRegisterFile(const char* file , DeviceOperations* ops);



