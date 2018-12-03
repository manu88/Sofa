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


int handle_chdir(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	size_t pathSize = seL4_GetMR(1);

	int error = 0;
	char* str = malloc(pathSize+1);

	if ( str == NULL)
	{
		error = -ENOMEM;
	}
	else
	{
		for(int i=0; i< pathSize;i++)
		{
			str[i] = seL4_GetMR(2+ i);
		}
		str[pathSize] = 0;

//		printf("handle_chdir request '%s' pathSize %li from pid %i\n", str , pathSize , senderProcess->_pid);

		Inode* newPath = FileServerGetINodeForPath( str  , senderProcess->currentDir);

		if (newPath == NULL)
		{
			error = -ENOENT;
		}
		else if (newPath->type != INodeType_Folder )
		{
			error = -ENOTDIR;
		}
		else
		{
			error = 0;
			senderProcess->currentDir = newPath;
//			printf("chdir '%s' is valid ('%s')\n" , str , senderProcess->currentDir->name);

		}
		if (str)
		{
	    		free(str);
		}
	}

//	printf("chdir will return %i\n",error);
	message = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0,__SOFA_NR_chdir);
	seL4_SetMR(1 , error);
	seL4_Reply( message );
	return 0;
}

