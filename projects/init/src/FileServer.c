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

#define MAX_SIZE_HASH 10


typedef struct
{
    Inode _rootNode;
    
} _FileServerContext;


static _FileServerContext _fsContext;

/*
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

*/

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

int FileServerRegisterHandler( FileServerHandler* handler , const char* forPath)
{
    
    return 0;
}

int FileServerRemoveHandler( FileServerHandler* handler , const char* atPath)
{

    return 0;
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
        /*
    if (pathname[0] != '/')
    {
        printf("FileServerOpen : '%s' only absolute path works for now \n" , pathname);
        *error = -ENODEV;
        return NULL;
    }
    
    
    return NULL;
    
    int pos = GetSecondSlash(pathname);
    
    if (pos == -1)
    {
        *error = -EINVAL;
        return NULL;
    }
    
    
    char* baseDir =  strndup(pathname, (unsigned long) pos);
    
    if (!baseDir)
    {
        *error = -ENOMEM;
        return NULL;
    }
    
    uint32_t key = StringHash(baseDir);

    free(baseDir);
    baseDir = NULL;
    
    FileServerHandler* handler = chash_get(&_fsContext._handlers, key);
    
    if (handler && MatchPrefix( handler->prefix, pathname ) )
    {
        const char* realPath = pathname + strlen(handler->prefix);
        
        Inode* retNode = handler->onOpen(context , realPath , flags , error);
        
        if(retNode)
        {
            
            retNode->name = strdup(realPath);
            InodeAddChild(&handler->inode, retNode);
        }
        return retNode;
    }
    
    return NULL;
     */
}









int FileServerHandlerInit(FileServerHandler* hander , const char* name)
{
    assert(hander);
    
    
    if(InodeInit(&hander->inode ,INodeType_Folder , name) )
    {
        return 1;
    }
    return 0;
}


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
