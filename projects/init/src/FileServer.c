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

static FileServerHandler _handler = {0};



static int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int FileServerInit()
{
    return 1;
}


int FileServerRegisterHandler( FileServerHandler* handler)
{
    if(_handler.perfix != NULL)
    {
        return 0;
    }
    
    _handler = *handler;
    return 1;
}


Inode* FileServerOpen(InitContext* context , const char*pathname , int flags)
{
    if(_handler.perfix && prefix(_handler.perfix, pathname))
    {
        const char* realPath = pathname + strlen(_handler.perfix);
        return _handler.onOpen(context , realPath , flags);
    }
    
    return NULL;
}
