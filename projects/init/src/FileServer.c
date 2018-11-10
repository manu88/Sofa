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
#include <libgen.h>

typedef struct
{
    Inode _rootNode;
    
} _FileServerContext;


static _FileServerContext _fsContext;


int FileServerInit()
{
    memset(&_fsContext, 0, sizeof(_FileServerContext));
    
    
    if( InodeInit(&_fsContext._rootNode , INodeType_Folder ,"") == 0)
    {
        
        return 0;
    }
    
    //_fsContext._rootNode.name = "";//strdup( "" );
    _fsContext._rootNode._parent = NULL;

    return 1;
}

Inode* FileServerGetRootNode()
{
    return &_fsContext._rootNode;
}

Inode* FileServerOpen(InitContext* context , const char*pathname , int flags , int*error)
{
    Inode* n = FileServerGetINodeForPath(pathname);
    
    *error = -ENOENT;
    if (n )
    {
        *error = n->inodeOperations->Open(n , flags);
        
        if (*error == 0)
        {
            InodeRetain(n);
            return n;
        }
        
    }

    return NULL;
}
/*
int FileServerHandlerInit(FileServerHandler* hander , const char* name)
{
    assert(hander);
    
    
    if(InodeInit(&hander->inode ,INodeType_Folder , name) )
    {
        return 1;
    }
    return 0;
}
*/


Inode* FileServerGetINodeForPath( const char* path_)
{
    if (strlen(path_) == 0)
        return NULL;
    
    
    char* path = strdup(path_);
    /*
    char* dir = dirname(path);
    printf("Dir '%s'\n" , path);
     */
    Inode* ret = &_fsContext._rootNode;
    
    static const char delim[] = "/";
    
    char* token = strtok(path, delim);
    
    while (token != NULL)
    {
        //printf("Token '%s'\n", token);
        
        ret = InodeGetChildByName(ret ,token);
        if(!ret)
        {
            free(path);
            return NULL;
        }
        token = strtok(NULL, delim);
    }
    
    free(path);
    return ret;
}


int FileServerAddNodeAtPath( Inode* node,const char* path)
{
    Inode* n = FileServerGetINodeForPath(path);
    
    if (n)
    {
        return InodeAddChild(n, node);
    }
    return 0;
}
