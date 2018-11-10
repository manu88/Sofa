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
#include <assert.h>
#include "StringOperations.h"

typedef struct
{
    Inode _devNode;
    
    INodeOperations _devOperations;
    
} _DevServerContext;

static _DevServerContext _context;


void DevChildRemoved( Inode*  lastParent , Inode* node)
{
    assert( node->_parent == NULL);
    assert(lastParent == &_context._devNode );
    
    InodeRelease(node);
}

int DevServerInit()
{
    if( InodeInit(&_context._devNode, INodeType_Folder, "dev"))
    {
        _context._devOperations.ChildRemoved = DevChildRemoved;
        _context._devNode.inodeOperations = &_context._devOperations;
        return 1;
    }
    
    return 0;
}

Inode* DevServerGetInode()
{
    return &_context._devNode;
}

int DevServerRegisterFile(Inode* node)
{
    if( InodeAddChild( &_context._devNode, node) )
    {
        InodeRetain( node);
        return 1;
    }
    
    return 0;
}

int DevServerRemoveFile( Inode* node)
{
    // node released in DevChildRemoved callback
    return InodeRemoveChild(&_context._devNode, node);
}
