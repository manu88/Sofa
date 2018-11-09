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
//  FileServer.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include "FileServer.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <data_struct/chash.h>
#include "StringOperations.h"


#define MAX_SIZE_HASH 10


typedef struct
{
    chash_t _handlers;
    
    Inode _rootNode;
    
} _FileServerContext;


static _FileServerContext _fsContext;


static int MatchPrefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

// only works if str starts with '/' !!
static int GetSecondSlash(const char* str)
{
    assert(str[0] == '/');
 
    char * ret = strchr(str+1,'/');
    if(ret == NULL)
    {
        return -1;
    }
    
   return (int) (ret-str+1);
}



int FileServerInit()
{
    memset(&_fsContext, 0, sizeof(_FileServerContext));
    
    
    if( InodeInit(&_fsContext._rootNode) == 0)
    {
        return 0;
    }
    _fsContext._rootNode._parent = NULL;
    
    chash_init(&_fsContext._handlers, MAX_SIZE_HASH);
    
    return _fsContext._handlers.table != NULL;
}

Inode* FileServerGetRootNode()
{
    return &_fsContext._rootNode;
}

int FileServerRegisterHandler( FileServerHandler* handler , const char* forPath)
{
    uint32_t key = StringHash(handler->prefix);
    
    if( chash_get(&_fsContext._handlers, key))
    {
        return 0;
    }

    if(chash_set( &_fsContext._handlers, key, handler ) == 0)
    {
        handler->inode._parent = &_fsContext._rootNode;
        return InodeAddChild(&_fsContext._rootNode , &handler->inode);
    }
    return 0;
}


Inode* FileServerOpen(InitContext* context , const char*pathname , int flags , int*error)
{
    
    if (pathname[0] != '/')
    {
        printf("FileServerOpen : '%s' only absolute path works for now \n" , pathname);
        *error = -ENODEV;
        return NULL;
    }
    
    int pos = GetSecondSlash(pathname);
    
    if (pos == -1)
    {
        *error = -EINVAL;
        return NULL;
    }
    
    
    char* baseDir =  strndup(pathname, (unsigned long) pos);
    
    uint32_t key = StringHash(baseDir);

    free(baseDir);
    baseDir = NULL;
    
    FileServerHandler* handler = chash_get(&_fsContext._handlers, key);
    
    if (handler && MatchPrefix( handler->prefix, pathname ) )
    {
        const char* realPath = pathname + strlen(handler->prefix);
        return handler->onOpen(context , realPath , flags , error);
    }
    
    return NULL;
}









int FileServerHandlerInit(FileServerHandler* hander , const char* name)
{
    assert(hander);
    
    
    if(InodeInit(&hander->inode))
    {
        return 1;
    }
    return 0;
}
