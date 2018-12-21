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

int handle_unlink(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

        const int msgLen = seL4_MessageInfo_get_length(message);
        assert(msgLen > 1);

        int ret= 0;

        char* pathname = malloc(sizeof(char)*msgLen );
        if(!pathname)
        {
                ret = -ENOMEM;
                // return some error
        }
        else 
        {
                ret = 0;
        }

        for(int i=0;i<msgLen-1;++i)
        {
            pathname[i] =  (char) seL4_GetMR(1+i);
        }

        pathname[msgLen-1] = 0;

	Inode* node = FileServerGetINodeForPath(pathname , senderProcess->currentDir);

	if (node)
	{
		printf("Process (pid %i) %lu ask to remove %lu (%s)\n" ,senderProcess->_pid, senderProcess->_identity.uid , node->_identity.uid , pathname);
		if (InodeIdentityHasWritePermissions(node , &senderProcess->_identity))
		{
		    if (FileServerUnlinkNode(node))
		    {
			ret = 0;
		    }
		}
		else 
		{
		    ret = -EPERM;
		}
	}
	else 
	{
		ret = -ENOENT;
	}
	free(pathname);


        message = seL4_MessageInfo_new(0, 0, 0, 2);

        seL4_SetMR(0, __SOFA_NR_unlink );
        seL4_SetMR(1, ret);

        seL4_Reply( message );


	return 0;
}

