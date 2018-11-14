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

#include "../SysCalls.h"
#include <SysCallNum.h>
#include <assert.h>
#include "../FileServer.h"
#include <fcntl.h>



int handle_lseek(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int fd = seL4_GetMR(1);
	const off_t offset = seL4_GetMR(2);
	const int whence = seL4_GetMR(3);

	//Lseek
	ssize_t ret= -ENOSYS;
	
	Inode* node = ProcessGetNode(senderProcess , fd);
	if(node)
        {
		ret = node->operations->Lseek(node, offset , whence);
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0, __SOFA_NR_lseek );
	seL4_SetMR(1, ret);

	seL4_Reply( message );

	return 0;
}


int handle_read(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int msgLen = seL4_MessageInfo_get_length(message);

	assert(msgLen == 4);
	const int fd = seL4_GetMR(1);
	const size_t count = seL4_GetMR(2);
	const int readMode = seL4_GetMR(3); //  1 = read, 2 = getdents64

	Inode* node = ProcessGetNode(senderProcess , fd);
	
	char *buf = NULL;
	ssize_t ret = 0;

	if(node)
	{

		if (readMode == 1 && node->type == INodeType_Folder) //'normal' read
		{
			ret = -EISDIR;
		}
		
		if (ret == 0)
		{
			buf = malloc(count);
			if (!buf)
			{
				ret = -ENOMEM;
			}
		}

		if(buf && ret == 0)
		{
			assert(node->operations);
			assert(node->operations->Read);
			ret = node->operations->Read(node ,buf , count);

		}
		const ssize_t msgLen = 2 + ( ret >0? ret : 0);
		assert(msgLen >= 2);
		message = seL4_MessageInfo_new(0, 0, 0, msgLen);
		
		seL4_SetMR(0, __SOFA_NR_read );
        	seL4_SetMR(1, ret);

		if (ret > 0)
		{
			for(int i=0; i< ret ; ++i)
			{
				seL4_SetMR(2 + i, buf[i]);
			}
		}
		seL4_Reply( message );

		if (buf)
		{
			free(buf);
		}
		return 0;
	}

	assert(0);
	return 0;
}

int handle_write(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

	// 0 : sysNum
        // 1 : fd
        // 2 : size

	const int fd =  seL4_GetMR(1);
	const size_t count = seL4_GetMR(2);

	int  ret= -ENOSYS;

	Inode* node = ProcessGetNode(senderProcess , fd);
        if(node)
        {
		char* buf = malloc( count);
		assert(buf); // TODO handle error
		memset(buf,0,count);

		for(int i =0;i<count;++i)
		{
			buf[i] = seL4_GetMR(3+i);
		}


		ret = node->operations->Write(node, buf , count);
		free(buf);
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);
	
	seL4_SetMR(0, __SOFA_NR_write );
        seL4_SetMR(1, ret);

	seL4_Reply( message );

        return 0;
}


int handle_open(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

	const int msgLen = seL4_MessageInfo_get_length(message);
	assert(msgLen > 2);

	char* pathname = malloc(sizeof(char)*msgLen -1);
	if(!pathname)
	{
		// return some error
	}

	int flags = seL4_GetMR(1);

	for(int i=0;i<msgLen-2;++i)
        {
            pathname[i] =  (char) seL4_GetMR(2+i);
        }

	pathname[msgLen-2] = 0;

	printf("handle_open request '%s'\n" , pathname);
	int ret= -ENOSYS;

	Inode* node =  FileServerOpenRelativeTo( pathname , senderProcess->currentDir,flags , &ret);

	if (node == NULL &&  flags & O_CREAT)
	{
		printf("handle_open got O_CREAT flag\n");
		
		node = InodeAlloc( INodeType_File , pathname );
		if (!node)
		{
			ret = -EPERM;
		}

		if(InodeAddChild( senderProcess->currentDir , node))
		{
			ret = 0;
		}
	
	}
	if(node && ret == 0)
	{
		printf("No error\n");
		ret = ProcessAppendNode(senderProcess , node);
	}
	else 
	{
		printf("Unable to open '%s' \n" , pathname);
	}

	printf("handle_open ret%i \n", ret);

	message = seL4_MessageInfo_new(0, 0, 0, 2);

	seL4_SetMR(0, __SOFA_NR_open );
	seL4_SetMR(1, ret);

	seL4_Reply( message );


//	free(pathname);
        return 0;
}


int handle_close(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int fd =  seL4_GetMR(1);
	printf("handle_close on fd %i\n" , fd);
	
	int error = 0;

	message = seL4_MessageInfo_new(0, 0, 0, 2);
        seL4_SetMR(0, __SOFA_NR_close);
	seL4_SetMR(1, error);

	seL4_Reply( message );

	return 0;
}


