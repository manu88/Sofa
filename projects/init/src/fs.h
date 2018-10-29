//
//  fs.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once

#include <stdint.h> //ssize_t

struct _inode;

struct _fileOperations
{
    ssize_t (*Read) (struct _inode *, char*  , size_t);
    ssize_t (*Write) (struct _inode *,  const char* ,size_t);
};

typedef struct _fileOperations FileOperations;

struct _inode
{
    const struct FileOperations *operations;
};

typedef struct _inode Inode;
