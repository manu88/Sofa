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

typedef struct
{
    FileOperations fileOps;
    ssize_t (*OpenDevice) (struct _inode *, char*  , size_t);
    
} DeviceOperations;

FileServerHandler* getDevServerHandler(void);

int DevServerInit(void);

int DevServerRegisterFile(const char* file , DeviceOperations* ops);



