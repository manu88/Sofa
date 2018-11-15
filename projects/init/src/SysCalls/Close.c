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


int handle_close(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int fd =  seL4_GetMR(1);
	

	int error = 0;

	if (ProcessGetNumFDs(senderProcess) <= fd )
	{
		error = -EBADF;
	}
	else// if (ProcessRemoveNode(senderProcess , fd) == 0 )
	{

		error = -EBADF;
		Inode* node = ProcessGetNode(senderProcess , fd);

		if (node)
		{	
			error = 0;
			node->pos = 0;
			assert(ProcessRemoveNode(senderProcess , fd) );
		}

	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);
        seL4_SetMR(0, __SOFA_NR_close);
	seL4_SetMR(1, error);

	seL4_Reply( message );

	return 0;
}


