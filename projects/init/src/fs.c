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
#include <stdio.h>


#include "StringOperations.h"


//#define MAX_CHILD_NODE 10
Inode* InodeAlloc(INodeType type, const char* name)
{
    Inode* n = malloc(sizeof(Inode));
    if (n && InodeInit(n , type,name))
    {

        return n;
    }
    
    return NULL;
}

int InodeInit(Inode* node ,INodeType type , const char* name)
{
    memset(node , 0, sizeof(Inode));
    
    node->refCount = 1;
    node->type = type;
    node->name = name;
    
    node->children = NULL;
    
    
    //chash_init(&node->_children, MAX_CHILD_NODE);
    return 1;
}


void InodeRetain(Inode* node)
{
    node->refCount++;
}

int InodeRelease(Inode* node)
{
    if( --node->refCount == 0)
    {
        //chash_release(&node->_children);
        return 1;
    }
    
    return 0;
    //free(node);
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


int INodeOperations_NoOpen (struct _inode *node, int flags)
{
    UNUSED_PARAMETER(node);
    UNUSED_PARAMETER(flags);
    
    return -EPERM;
}


int InodeAddChild( Inode* root , Inode* child)
{
    if (root->type != INodeType_Folder)
    {
        return 0;
    }
    
    Inode* n = NULL;
    HASH_FIND_STR(root->children, child->name, n);
    
    if( n != NULL)
    {
        return 0;
    }
    
    HASH_ADD_STR(root->children, name, child);
    
    child->_parent = root;
    
    
    return InodeGetChildByName(root, child->name) != NULL;

}

int InodeRemoveChild(Inode* node ,Inode* child )
{
    Inode* n =NULL;
    HASH_FIND_STR(node->children, child->name, n);
   
    if( n)
    {
        
        return InodeRemoveFromParent(child);
        //HASH_DEL(node->children, child);
        

        //return 1;
    }
    return 0;
}
                                                
int InodeRemoveFromParent(Inode* node )
{
    if (node->_parent == NULL)
    {
        return 0;
    }
    
    Inode* lastParent = node->_parent;
    
    HASH_DEL(lastParent->children, node);
    
    node->_parent = NULL;
    if (lastParent->inodeOperations && lastParent->inodeOperations->ChildRemoved)
    {
        lastParent->inodeOperations->ChildRemoved( lastParent , node);
    }
    return 1;
}

size_t InodeGetChildrenCount(const Inode* node)
{
    return HASH_COUNT(node->children);
    /*
    node->_children.
    InodeList* entry = NULL;
    InodeList* entry_temp = NULL;
    
    size_t c = 0;
    LIST_FOREACH_SAFE(entry, &node->children, entries ,entry_temp )
    {
        c++;
    }
    
    return c;
     */
    
    return 0;
}


ssize_t InodeGetAbsolutePath(const Inode* node, char* b, size_t maxSize)
{
    assert(node);
    
    Inode* prev = (Inode*) node;
    
    ssize_t strIndex = 0;
    
    memset(b, 0, maxSize);
    
    int accum = 0;
    while (prev != NULL)
    {
        if (strIndex >= maxSize)
        {
            return -1;
        }
        //printf("prepend path '%s' to '%s'\n",prev->name , b);
        
        if (prev->type == INodeType_Folder)
        {
            StringPrepend(b, "/");
            strIndex += 1;
        }
        
        StringPrepend(b, prev->name);
        
        strIndex += strlen(prev->name);
        
        if(++accum > MAX_PATH_LOOKUP)
        {
            return -ELOOP;
        }
        
        if (prev->_parent == prev)
        {
            prev = NULL;
        }
        else
        {
            prev = prev->_parent;
        }
        
        //assert(node->_parent != node);
    }
    
    return strIndex;
}


Inode* InodeGetChildByName( const Inode* node , const char* name)
{
    Inode* el = NULL;
    Inode* tmp = NULL;
    HASH_ITER(hh, node->children, el, tmp)
    {
        if (strcmp(el->name, name) == 0)
        {
            return el;
        }
    }
    /*
    LIST_FOREACH_SAFE(entry, &node->children, entries ,entry_temp )
    {
        if (strcmp(entry->node->name, name) == 0)
        {
            return entry->node;
        }
    }
    */
    return NULL;
}

static void _printNode(const Inode* node , int indent)
{
    Inode* child = NULL;
    Inode* tempChild = NULL;
    
    InodeForEachChildren(node, child, tempChild)
    {
        for(int i =0;i<indent;i++)
            printf("\t");
        
        printf("'%s' %s \n" , child->name , child->type == INodeType_Folder? "Folder":"File");
        if (child->type == INodeType_Folder)
        {
            _printNode(child , indent + 1);
        }
    }
}

void InodePrintTree(const Inode* node)
{
    
    printf("-- Start tree -- \n");
    printf("'%s'\n" , node->name);
    _printNode(node , 1);
    
    
    printf("-- End tree -- \n");
}
