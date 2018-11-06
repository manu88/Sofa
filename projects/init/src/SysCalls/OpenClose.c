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



int handle_lseek(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int fd = seL4_GetMR(1);
	const off_t offset = seL4_GetMR(2);
	const int whence = seL4_GetMR(3);

	printf("Init : Got a lseek request\n");


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

	
	const int fd = seL4_GetMR(1);
	const size_t count = seL4_GetMR(2);

	Inode* node = ProcessGetNode(senderProcess , fd);
	if(node)
	{
		char buf[4];
		ssize_t ret = node->operations->Read(node ,buf , count);

		message = seL4_MessageInfo_new(0, 0, 0, 2 + ret);
		
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

		for(int i =0;i<count;++i)
		{
			buf[i] = seL4_GetMR(3+i);
		}


		ret = node->operations->Write(node, buf , count);

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

	pathname[msgLen] = '0';

	int ret= -ENOSYS;

	Inode* node =  FileServerOpen(context , pathname,flags , &ret);

	if(node && ret == 0)
	{
		printf("No error\n");
		ret = ProcessAppendNode(senderProcess , node);
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);

	seL4_SetMR(0, __SOFA_NR_open );
	seL4_SetMR(1, ret);

	seL4_Reply( message );


//	free(pathname);
        return 0;
}


int handle_close(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        return 0;
}


