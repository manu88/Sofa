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


int handle_execve(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    const int msgLen = seL4_MessageInfo_get_length(message);
    assert(msgLen > 0);
    char* filename = malloc(sizeof(char)*msgLen );/* msglen minus header + 1 byte for NULL byte*/
    assert(filename);
    
    for(int i=0;i<msgLen-1;++i)
    {
        filename[i] =  (char) seL4_GetMR(1+i);
    }

    filename[msgLen] = '0';

    printf("Init : Execve size %i filename '%s'\n", msgLen , filename);

    /**/
    Process *newProcess = ProcessAlloc();
    assert(newProcess);
        
    newProcess->currentDir = senderProcess->currentDir;

    int  error = ProcessTableAddAndStart(context,  newProcess,filename, context->ep_cap_path ,senderProcess, seL4_MaxPrio);
    assert(error == 0);

    /**/
    free(filename);
    
    seL4_SetMR(0,__SOFA_NR_execve);
    seL4_SetMR(1, newProcess->_pid/*  -ENOSYS*/ ); // return code is the new pid
    seL4_Reply( message );
        return 0;
}
