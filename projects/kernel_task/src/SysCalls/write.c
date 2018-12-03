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
#include <assert.h>
#include "../FileServer.h"
#include <fcntl.h>


int handle_write(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
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
