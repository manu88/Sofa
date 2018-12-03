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


int handle_fcntl(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

	int fd  = seL4_GetMR(1);
	int cmd = seL4_GetMR(2);
	
	int ret = 0;

	switch(cmd)
	{

		case F_SETFD:
		{
			int flag = seL4_GetMR(3);
			ret  = 0;
		}
			break;

		case F_GETFD:
			ret = 0;
			if (ProcessGetNumFDs(senderProcess) <= fd)
			{
				ret = -1;
			}
			break;

		default:
		assert(0);
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0 ,__SOFA_NR_fcntl);
	seL4_SetMR(1 , ret);

	seL4_Reply( message );
	return 0;
}

