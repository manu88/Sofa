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
#include <cpio/cpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <fcntl.h>
static Inode*  CpioOpen(void* context, const char*pathname ,int flags , int *error);

extern char _cpio_archive[];

static const char cpioBaseName[] = "/cpio/";


static FileServerHandler _handler = { "/cpio/" ,   CpioOpen};


static ssize_t CPIORead (struct _inode *node, char*buf  , size_t bufSize);
static ssize_t CPIOWrite(struct _inode *node,  const char*buf ,size_t bufSize);
static ssize_t CPIOLseek (struct _inode *node, size_t offset, int whence);


static const FileOperations cpioOps = 
{
	CPIORead,
	CPIOWrite,
	CPIOLseek,
};


FileServerHandler* getCPIOServerHandler(void)
{
	return &_handler;
}



int CPIOServerInit()
{
	struct cpio_info info;

	if(cpio_info(_cpio_archive , &info) != 0)
	{
		return 0;
	}

	if( FileServerHandlerInit(&_handler, "dev") == 0)
	{
		return 0;
	}
/*
	char **buf = malloc( info.file_count);

	for(int i=0;i<info.file_count;i++)
	{
		buf[i] = malloc(info.max_path_sz);
	}

	cpio_ls(_cpio_archive , buf, info.file_count);

	printf("Got %u files \n",info.file_count);

	for(int i=0;i<info.file_count;i++)
        {
		printf("File '%s'\n" , buf[i]);
	}

*/

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

static Inode*  CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
	printf("CpioOpen '%s' , flags %i\n" , pathname , flags);

	// O_RDONLY, O_WRONLY ou O_RDWR
	if (flags != O_RDONLY)
	{
		*error = -EPERM;
		return NULL;
	}
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
}


static ssize_t CPIORead (struct _inode *node, char* buf , size_t size)
{
	printf("CPIO READ request %lu bytes pos %lu size max %lu\n", size , node->pos , node->size);
	
	if (size > node->size - node->pos)
	{
		size = node->size - node->pos;
	}

	memcpy(buf , node->userData + node->pos , size);
	node->pos += size;

	return size;
}

static ssize_t CPIOWrite(struct _inode *node,  const char* buf ,size_t size)
{
	return 0;
}

static ssize_t CPIOLseek (struct _inode *node, size_t offset, int whence)
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
