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


int handle_mkdir(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	// 0 sys call num
	// 1 mode
	// 2  string length
 	// 3 .. string length : string content

        const int msgLen = seL4_MessageInfo_get_length(message);
        assert(msgLen >= 3);

	const mode_t mode = seL4_GetMR(1);

        const size_t pathLen = seL4_GetMR(2);
        int ret= 0;

        char* pathname = malloc(sizeof(char)*pathLen);
        if(!pathname)
        {
                ret = -ENOMEM;
        }
        else 
        {
                ret = 0;
        }

        for(int i=0;i<pathLen;++i)
        {
            pathname[i] =  (char) seL4_GetMR(3+i);
        }

        pathname[pathLen] = 0;

	Inode* newFolder = FileServerCreateNode(pathname , INodeType_Folder , senderProcess->currentDir );
	if (newFolder == NULL)
	{
		ret = -EPERM;
	}

	printf("Handle mkdir mode %i path '%s' ret %i \n" , mode , pathname, ret);
        message = seL4_MessageInfo_new(0, 0, 0, 2);

        seL4_SetMR(0, __SOFA_NR_mkdir );
        seL4_SetMR(1, ret);

        seL4_Reply( message );


        free(pathname);
        return 0;
}

