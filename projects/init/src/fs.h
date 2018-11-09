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

struct _inode;


typedef struct _inodeList InodeList;
struct _inodeList
{
    struct _inode *node;
    LIST_ENTRY(_inodeList) entries;
};

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

struct _inode
{
    struct _inode* _parent;
    LIST_HEAD(listhead2, _inodeList) children;
    
    const FileOperations *operations;

    size_t pos;
    size_t size;
    void* userData;
};

typedef struct _inode Inode;


Inode* InodeAlloc(void) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int InodeInit(Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

void InodeRelease(Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;


size_t InodeGetChildrenCount(const Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int InodeAddChild( Inode* root , Inode* child) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
