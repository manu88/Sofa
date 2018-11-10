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
//  fs.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once

//#include <stdint.h> //ssize_t
#include <sys/types.h>
#include "Sofa.h"
#include "TimerWheel/queue.h"
#include <data_struct/chash.h>
#include "uthash.h"

typedef enum
{
    INodeType_File   = 1,
    INodeType_Folder = 2,
} INodeType;
struct _inode;


struct _fileOperations
{
    ssize_t (*Read) (struct _inode *, char*  , size_t);
    ssize_t (*Write) (struct _inode *,  const char* ,size_t);

    ssize_t  (*Lseek) (struct _inode *, size_t, int);

};

// default method returning EPERM 
ssize_t FileOperation_NoRead (struct _inode *node, char*buf  , size_t len);
ssize_t FileOperation_NoWrite(struct _inode *node,  const char* buf ,size_t len);
ssize_t FileOperation_NoLseek (struct _inode *node, size_t off, int whence);

typedef struct _fileOperations FileOperations;


struct _iNodeOperations
{
    int (*Open) (struct _inode *, int flags);
    int (*Close) (struct _inode *);
    // called on parent when a child is removed
    void (*ChildRemoved)(struct _inode* lastParent , struct _inode* node);
};

int INodeOperations_NoOpen (struct _inode *node, int flags);

typedef struct _iNodeOperations INodeOperations;

struct _inode
{
    
    
    size_t refCount;
    INodeType type;
    const char* name;
    
    //chash_t _children;
    
    const FileOperations  *operations;
    const INodeOperations *inodeOperations;

    size_t pos;
    size_t size;
    void* userData;
    
    struct _inode* _parent;
    struct _inode* children;
    UT_hash_handle hh;
    
    
};

typedef struct _inode Inode;


Inode* InodeAlloc(INodeType type,const char* name) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int InodeInit(Inode* node , INodeType type , const char* name) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

void InodeRetain(Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
// returns 1 if node can be freed (ie refCount is 0 after decrement)
int InodeRelease(Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

size_t InodeGetChildrenCount(const Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int InodeAddChild( Inode* root , Inode* child) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int InodeRemoveChild(Inode* node ,Inode* child ) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int InodeRemoveFromParent(Inode* node ) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

Inode* InodeGetChildByName( const Inode* node , const char* name) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

ssize_t InodeGetAbsolutePath(const Inode* node, char* b, size_t maxSize) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
