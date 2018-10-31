//
//  DevServer.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include "DevServer.h"
#include <stdio.h>

static Inode*  _DevOpen(void* context, const char*pathname ,int flags , int *error);


static FileServerHandler _handler = { "/dev/" ,   _DevOpen};

FileServerHandler* getDevServerHandler(void)
{
    return &_handler;
}

int DevServerInit()
{
    return 1;
}


static Inode*  _DevOpen(void* context, const char*pathname ,int flags , int *error)
{
    printf("dev open request for '%s' \n" , pathname);
    return NULL;
}
