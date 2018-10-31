//
//  DevServer.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <string.h>

#include "DevServer.h"
#include <stdio.h>
#include <data_struct/chash.h>
#include "StringOperations.h"

#define MAX_SIZE_HASH 10

static Inode*  _DevOpen(void* context, const char*pathname ,int flags , int *error);


typedef struct
{
    FileServerHandler _handler;// = { "/dev/" ,   _DevOpen};
    chash_t _files;
    
} _DevServerContext;

static _DevServerContext _context =
{
    { "/dev/" ,   _DevOpen}
};

FileServerHandler* getDevServerHandler(void)
{
    return &_context._handler;
}

int DevServerInit()
{
    chash_init(&_context._files, MAX_SIZE_HASH);
    return _context._files.table != NULL;
}


static Inode*  _DevOpen(void* context, const char*pathname ,int flags , int *error)
{
    printf("dev open request for '%s' \n" , pathname);
    
    uint32_t key = StringHash(pathname);
    
    DeviceOperations* ops =  chash_get(&_context._files, key);
    
    if (ops)
    {
        *error = 0;
        return ops->OpenDevice(ops , flags);
        
    }
    *error = -ENOENT;
    return NULL;
}


int DevServerRegisterFile(const char* file , DeviceOperations* ops)
{
    if (file[0] == '/' || ops == NULL)
    {
        return 0;
    }
    
    uint32_t key = StringHash(file);
    
    if( chash_get(&_context._files, key))
    {
        return 0;
    }
    
    return  chash_set( &_context._files, key, ops ) == 0;
}
