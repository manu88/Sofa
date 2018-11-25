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

#include "FileServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <fcntl.h>
#include <errno.h>

#ifndef __APPLE__
#include <cpio/cpio.h>


extern char _cpio_archive[];
#endif

static int CpioOpen (Inode * node , int flags);
static int     CpioClose (struct _inode *);


static ssize_t CpioRead (struct _inode *node, char*buf  , size_t bufSize);
static ssize_t CpioWrite(struct _inode *node,  const char*buf ,size_t bufSize);
static ssize_t CpioLseek (struct _inode *node, size_t offset, int whence);

typedef struct
{
    Inode node;
    
    FileOperations  operations;
    INodeOperations inodeOperations;
    
} CPIOContext;

static CPIOContext _context;

/*
static const FileOperations cpioOps = 
{
	CPIORead,
	CPIOWrite,
	CPIOLseek,
};
*/
/*
FileServerHandler* getCPIOServerHandler(void)
{
	return &_handler;
}
*/


int CPIOServerInit()
{

    
    if (InodeInit(&_context.node, INodeType_Folder, "cpio") == 0)
    {
        return 0;
    }

    _context.inodeOperations.Open  = CpioOpen;
    _context.inodeOperations.Close = CpioClose;

    _context.operations.Lseek = CpioLseek;
    _context.operations.Write = CpioWrite;
    _context.operations.Read  = CpioRead;

//    _context.node.inodeOperations = &_context.inodeOperations;
//    _context.node.operations      = &_context.operations;

#ifndef __APPLE__
    struct cpio_info info;
    
    if(cpio_info(_cpio_archive , &info) != 0)
    {
        return 0;
    }

    // populate nodes
    char **buf = malloc( info.file_count);
    assert(buf);

    for(int i=0;i<info.file_count;i++)
    {
	buf[i] = malloc(info.max_path_sz );
	assert(buf[i]);
	memset(buf[i] , 0 , info.max_path_sz);

    }

    cpio_ls(_cpio_archive , buf, info.file_count);
//	printf("Got %u files \n",info.file_count);

    for(int i=0;i<info.file_count;i++)
    {
	assert(buf[i]);
	Inode* f = InodeAlloc(INodeType_File, buf[i]);
	if (f)
	{
		f->inodeOperations = &_context.inodeOperations;
		f->operations      = &_context.operations;
		InodeAddChild(&_context.node , f);
	}

//		printf("File '%s'\n" , buf[i]);
    }
#endif


/* TEST*/

/*
unsigned long fileSize = 0;
        void* dataContent = cpio_get_file(_cpio_archive , "hello.txt" , &fileSize);

	if( dataContent)
	{
		printf("Found Hello %lu bytes\n" , fileSize);
		printf("%.*s", fileSize, dataContent);
	}
*/
	return 1;
}


Inode* CPIOServerGetINode()
{
    return &_context.node;
}
static int CpioClose ( Inode * node)
{
    return 0;
}

//static Inode*  CpioOpen(void* context, const char*pathname ,int flags, int *error)

static int CpioOpen (Inode * node , int flags)
{
    /*
	//printf("CpioOpen '%s' , flags %i\n" , pathname , flags);

	// O_RDONLY, O_WRONLY ou O_RDWR
	if (flags != O_RDONLY)
	{
		*error = -EPERM;
		return NULL;
	}
     */
    
#ifndef __APPLE__ 
/*
    unsigned long fileSize = 0;

	void* dataContent = cpio_get_file(_cpio_archive , pathname , &fileSize);

	if(dataContent == NULL)
	{
		*error = -ENOENT;
		return NULL;
	}

	*error = 0;
	Inode* node = malloc(sizeof(Inode) );

	if (!node)
	{
		*error = -ENOMEM;
		return NULL;
	}

	printf("CPIO File found : size %lu\n", fileSize);
	*error = 0;
	node->size = fileSize;
	node->pos = 0;
	node->userData = dataContent;
	node->operations = &cpioOps;
	return node;
*/
    return 0;
#else
    return 0;
    
#endif
}


static ssize_t CpioRead (struct _inode *node, char* buf , size_t size)
{
#ifndef __APPLE__
	printf("CPIO READ request %lu bytes pos %lu size max %lu\n", size , node->pos , node->size);
	

	if (node->userData == NULL)
	{
		unsigned long fileSize = 0;
        void* dataContent = cpio_get_file(_cpio_archive , node->name , &fileSize);
		
        node->userData = dataContent;
		node->size = fileSize;
	}
	if (size > node->size - node->pos)
	{
		size = node->size - node->pos;
	}

	memcpy(buf , node->userData + node->pos , size);
	node->pos += size;

	return size;
#else
    return 0;
#endif
}

static ssize_t CpioWrite(struct _inode *node,  const char* buf ,size_t size)
{
	return 0;
}

static ssize_t CpioLseek (struct _inode *node, size_t offset, int whence)
{

	switch(whence)
	{
		case SEEK_SET:
		node->pos = offset;
		break;

		case SEEK_CUR:
		node->pos +=offset;
		break;

		case SEEK_END:
		node->pos = node->size + node->pos;
		break;

		default:
		return -EPERM;
	}

	return 0;
}
