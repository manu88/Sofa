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
    
    FileOperations defaultFSOps;
    
} _FileServerContext;


static _FileServerContext _fsContext;


int FileServerInit()
{
    memset(&_fsContext, 0, sizeof(_FileServerContext));
    
    
    _fsContext.defaultFSOps.Lseek = FileServer_DefaultLseek;
    _fsContext.defaultFSOps.Read = FileServer_DefaultRead;
    
    if( InodeInit(&_fsContext._rootNode , INodeType_Folder ,"") == 0)
    {
        
        return 0;
    }
    _fsContext._rootNode._parent = &_fsContext._rootNode;

    return 1;
}

Inode* FileServerGetRootNode()
{
    return &_fsContext._rootNode;
}


Inode* FileServerOpenRelativeTo( const char* pathname , const Inode* relativeTo , int flags , int *error) 
{
    Inode* n = FileServerGetINodeForPath(pathname , relativeTo);
    
    *error = -ENOENT;
    if (n )
    {
        if (n->inodeOperations && n->inodeOperations->Open)
        {
            *error = n->inodeOperations->Open(n , flags);
        }
        else
        {
            *error = FileServer_DefaultOpen(n, flags);
        }
        
        if (*error == 0)
        {
            if (n->operations == NULL)
            {
                n->operations = &_fsContext.defaultFSOps;
            }
            InodeRetain(n);
            return n;
        }
        
    }

    return NULL;

}

Inode* FileServerOpen( /*InitContext* context ,*/ const char*pathname , int flags , int*error)
{
    return FileServerOpenRelativeTo(pathname , NULL , flags , error);
/*
    Inode* n = FileServerGetINodeForPath(pathname , NULL);
    
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
 */
}

Inode* FileServerGetINodeForPath( const char* path_ , const Inode* relativeTo)
{
    if (strlen(path_) == 0)
        return NULL;

    char* path = strdup(path_);

    Inode* ret = relativeTo == NULL? &_fsContext._rootNode : (Inode*) relativeTo;

    static const char delim[] = "/";

    char* token = strtok(path, delim);
    
    // TODO : Temp workaround until better implementation :)
    if (token == NULL && strcmp(path, "/") == 0)
    {
        free(path);
        
        return FileServerGetRootNode();
    }

    while ( token != NULL )
    {

        if (strcmp(token, "..") == 0)
        {
            ret = ret->_parent;
        }
        else if (strcmp(token, ".") == 0)
        {
            ret = ret;
        }
        else
        {
            ret = InodeGetChildByName(ret ,token);
        }
        if(!ret)
        {
            free(path);
            return NULL;
        }
        token = strtok(NULL, delim);

    }
    /*
    char *last = strrchr(path, '/');
    if (last)
    {
        printf("Got a last '%s' \n" , last);
    }
    */
    
    free(path);
    return ret;
}


int FileServerAddNodeAtPath( Inode* node,const char* path)
{
    Inode* n = FileServerGetINodeForPath(path, NULL);
    
    if (n)
    {
        return InodeAddChild(n, node);
    }
    return 0;
}


int FileServer_DefaultOpen (Inode *node, int flags)
{
    return 0;
}

int FileServer_DefaultClose (Inode *node)
{
    return 0;
}


ssize_t FileServer_DefaultRead (Inode *node, char*buf  , size_t len)
{
    if (node->type == INodeType_Folder)
    {
        printf("FileServer_DefaultRead folder request \n");
    }
    return 0;
}

ssize_t FileServer_DefaultLseek (Inode *node, size_t off, int whence)
{
    return 0;
}
