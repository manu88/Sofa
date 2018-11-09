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
#include <assert.h>

Inode* InodeAlloc()
{
    Inode* n = malloc(sizeof(Inode));
    if (n && InodeInit(n))
    {
        
        LIST_INIT(&n->children);
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



int InodeAddChild( Inode* root , Inode* child)
{
    
    InodeList* entry = (InodeList*) malloc(sizeof(InodeList)) ;
    
    if(!entry)
    {
        return 0;
    }
    entry->node = child;
    
    LIST_INSERT_HEAD(&root->children, entry, entries);
    return 1;
}

size_t InodeGetChildrenCount(const Inode* node)
{
    
    InodeList* entry = NULL;
    InodeList* entry_temp = NULL;
    
    size_t c = 0;
    LIST_FOREACH_SAFE(entry, &node->children, entries ,entry_temp )
    {
        c++;
    }
    
    return c;
}
