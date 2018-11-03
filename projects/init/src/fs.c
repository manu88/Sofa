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
