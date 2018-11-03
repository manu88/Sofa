//
//  fs.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include "fs.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

Inode* InodeAlloc()
{
    Inode* n = malloc(sizeof(Inode));
    if (n && InodeInit(n))
    {
        return n;
    }
    
    return NULL;
}

int InodeInit(Inode* node)
{
    memset(node , 0, sizeof(Inode));
    return 1;
}

void InodeRelease(Inode* node)
{
    free(node);
}


ssize_t FileOperation_NoRead (struct _inode *node, char*buf  , size_t len)
{
    UNUSED_PARAMETER(node);
    UNUSED_PARAMETER(buf);
    UNUSED_PARAMETER(len);
    return -EPERM;
}
ssize_t FileOperation_NoWrite(struct _inode *node,  const char* buf ,size_t len)
{
    UNUSED_PARAMETER(node);
    UNUSED_PARAMETER(buf);
    UNUSED_PARAMETER(len);
    return -EPERM;
}

ssize_t FileOperation_NoLseek (struct _inode *node, size_t off, int whence)
{
    UNUSED_PARAMETER(node);
    UNUSED_PARAMETER(off);
    UNUSED_PARAMETER(whence);
    return -EPERM;
}
