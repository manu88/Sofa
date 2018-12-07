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


int handle_open(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        int flags = seL4_GetMR(1);

        const int msgLen = seL4_MessageInfo_get_length(message);
        assert(msgLen > 2);

        int ret= 0;

        char* pathname = malloc(sizeof(char)*msgLen -1);
        if(!pathname)
        {
                ret = -ENOMEM;
                // return some error
        }
        else 
        {
                ret = 0;
        }

        for(int i=0;i<msgLen-2;++i)
        {
            pathname[i] =  (char) seL4_GetMR(2+i);
        }

        pathname[msgLen-2] = 0;


        Inode* node =  FileServerOpenRelativeTo( pathname , senderProcess->currentDir,flags , &ret);

	if (node && (flags & O_EXCL) )
        {
        //        printf("handle_open O_EXCL flag set\n");
                ret = -EEXIST;
        }
        else if (node == NULL &&  (flags & O_CREAT) )
        {
                ret = 0;

         //       printf("Create node '%s' from currentDir '%s'\n" , pathname , senderProcess->currentDir->name);
                node = FileServerCreateNode(pathname, INodeType_File , senderProcess->currentDir);

                if (!node)
                {
                        ret = -EPERM;
                }

        }
        if(node && ret == 0)
        {
                ret = ProcessAppendNode(senderProcess , node);
        }
/*
	else 
        {
                printf("Unable to open '%s' err %i \n" , pathname , ret);
        }
*/

        message = seL4_MessageInfo_new(0, 0, 0, 2);

        seL4_SetMR(0, __SOFA_NR_open );
        seL4_SetMR(1, ret);

        seL4_Reply( message );


//      free(pathname);
        return 0;
}

