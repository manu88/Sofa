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
//  DevServer.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <string.h>
#include <errno.h>
#include "DevServer.h"
#include <stdio.h>

#include "StringOperations.h"

static Inode*  _DevOpen(void* context, const char*pathname ,int flags , int *error);


typedef struct
{
    //FileServerHandler _handler;// = { "/dev/" ,   _DevOpen};
    Inode _devNode;
    
    
} _DevServerContext;

static _DevServerContext _context;/* =
{
    { "/dev/" ,   _DevOpen}
};
*/
Inode* DevServerGetInode()
{
    return &_context._devNode;
}


int DevServerInit()
{
    return InodeInit(&_context._devNode, INodeType_Folder, "dev");
    
}


static Inode*  _DevOpen(void* context, const char*pathname ,int flags , int *error)
{
    /*
    printf("dev open request for '%s' \n" , pathname);
    
    uint32_t key = StringHash(pathname);
    
    DeviceOperations* ops =  chash_get(&_context._files, key);
    
    if (ops)
    {
        *error = 0;
        return ops->OpenDevice(ops , flags);
        
    }
    printf("_DevOpen : unable to open '%s'\n", pathname);

    *error = -ENOENT;
     */
    return NULL;
}


int DevServerRegisterFile(Inode* node)
{
    return InodeAddChild( &_context._devNode, node);
}
