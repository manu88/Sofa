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

struct _inode;

struct _fileOperations
{
    ssize_t (*Read) (struct _inode *, char*  , size_t);
    ssize_t (*Write) (struct _inode *,  const char* ,size_t);

    ssize_t  (*Lseek) (struct _inode *, size_t, int);

};

typedef struct _fileOperations FileOperations;

struct _inode
{
   const FileOperations *operations;

   size_t pos;
   size_t size;
   void* userData; 
};

typedef struct _inode Inode;
