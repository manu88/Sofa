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
#include "../ProcessTable.h"


int handle_execve(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    const int msgLen = seL4_MessageInfo_get_length(message);
    assert(msgLen > 0);
    const size_t strSize = msgLen-1;
    char* filename = malloc(sizeof(char)*strSize );/* msglen minus header + 1 byte for NULL byte*/
    assert(filename);
    
    for(int i=0;i<strSize;++i)
    {
        filename[i] =  (char) seL4_GetMR(1+i);
    }

    filename[strSize] = 0;

    long err = 0;

    Inode* nodeToExec = FileServerGetINodeForPath(filename ,  senderProcess->currentDir);

    free(filename);

    if(nodeToExec)
    {
	assert(nodeToExec->name);
        Process *newProcess = ProcessAlloc();
        assert(newProcess);

        newProcess->currentDir = senderProcess->currentDir;

        const size_t numFds = ProcessGetNumFDs(senderProcess);

        for (size_t i = 0; i < numFds;i++)
        {
	    ProcessAppendNode(newProcess , ProcessGetNode(senderProcess , i));
        }

        int  error = ProcessTableAddAndStart(context,  newProcess, nodeToExec->name, context->ep_cap_path ,senderProcess, seL4_MaxPrio);
        assert(error == 0);

        assert(ProcessGetParent(newProcess) == senderProcess);
        assert(ProcessGetNumChildren(senderProcess) > 0);
        assert(ProcessGetChildByPID(senderProcess , newProcess->_pid) == newProcess);

	err = newProcess->_pid;
    }
    else 
    {
	err = -ENOENT;
    }
    
    seL4_SetMR(0,__SOFA_NR_execve);
    seL4_SetMR(1, err );
    seL4_Reply( message );
        return 0;
}
