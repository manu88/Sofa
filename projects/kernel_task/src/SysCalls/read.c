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

