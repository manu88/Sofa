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
    
    chash_init(&_fsContext._handlers, MAX_SIZE_HASH);
    
    return _fsContext._handlers.table != NULL;
}


int FileServerRegisterHandler( FileServerHandler* handler)
{
    uint32_t key = StringHash(handler->perfix);
    
    if( chash_get(&_fsContext._handlers, key))
    {
        return 0;
    }

    return  chash_set( &_fsContext._handlers, key, handler ) == 0;
    /*
    if(_fsContext._handler.perfix != NULL)
    {
        return 0;
    }
    
    _fsContext._handler = *handler;
    return 1;
     */
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
    
    if (handler && MatchPrefix( handler->perfix, pathname ) )
    {
        const char* realPath = pathname + strlen(handler->perfix);
        return handler->onOpen(context , realPath , flags , error);
    }
    
    return NULL;
}
