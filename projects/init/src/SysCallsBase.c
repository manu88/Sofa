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

#include "SysCalls.h"
#include "Utils.h"
#include "ProcessTable.h"
#include "MainLoop.h" // UpdateTimeout
#include <SysCallNum.h>

int handle_getpid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    seL4_SetMR(1, senderProcess->_pid);
    seL4_Reply( message );

    return 0;
}

int handle_getppid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    seL4_SetMR(1, senderProcess->_parent->_pid );
    seL4_Reply( message );
    return 0;
}

int handle_exit(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    int exitStatus = seL4_GetMR(1);
    printf("Got exit from %i status %i\n", senderProcess->_pid , exitStatus);

    ProcessSignalStop( senderProcess);
    ProcessDoCleanup( senderProcess);
    if(!ProcessTableRemove( senderProcess))
    {
        printf("Unable to remove process!\n");
    }
    ProcessStop(context , senderProcess);
    ProcessRelease(senderProcess);

    printf("Init : Got %i processes \n" , ProcessTableGetCount() );
    return 0;
}

int handle_kill(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        seL4_Word pidToKill = seL4_GetMR(1);
    seL4_Word sigToSend = seL4_GetMR(2);
    
    printf("Received a request from %i to kill process %li with signal %li\n",senderProcess->_pid , pidToKill , sigToSend);
    seL4_SetMR(1, -ENOSYS ); // error for now
    seL4_Reply( message );

    return 0;
}


int handle_nanosleep(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

    int error = 0;
    seL4_Word millisToWait = seL4_GetMR(1);
    //printf("Process %i request to sleep %li ms\n" , senderProcess->_pid , millisToWait);

    TimerContext* timerCtx =(TimerContext*) malloc(sizeof(TimerContext));

    if(timerCtx)
    {

	if( TimerInit(&timerCtx->timer, timerCtx, 1/*Oneshot*/))
	{
		timerCtx->process = senderProcess;
		if(TimerWheelAddTimer(&context->timersWheel , &timerCtx->timer , millisToWait))
		{
			timerCtx->reply = get_free_slot(context);
			error = cnode_savecaller( context, timerCtx->reply );
			if(error == 0)
			{
				UpdateTimeout(context,millisToWait * NS_IN_MS);
				return 0;
			}
			else 
			{
				// TODO handle release of timerCtx->reply
				error = -ENOMEM;
				TimerWheelRemoveTimer(&context->timersWheel , &timerCtx->timer);
				free(timerCtx);
			}
		}
		else
		{
			free(timerCtx);
			error = -ENOMEM;
		}
	}
	else // TimerInit
	{
		free(timerCtx);
		error = -ENOMEM;
	}
    }
    else  // timerCtx malloc
    {
	error = -ENOMEM;
    }

    // sends a reponse on error
    if(error != 0)
    {
	seL4_SetMR(1,error);
	seL4_Reply( message );
    }
    
    // save the caller
    /*
    timerCtx->reply = get_free_slot(context);
    int error = cnode_savecaller( context, timerCtx->reply );
    assert(error == 0);

    assert(TimerInit(&timerCtx->timer, timerCtx, 1/));
    timerCtx->process = senderProcess;
    assert(TimerWheelAddTimer(&context->timersWheel , &timerCtx->timer , millisToWait));
    UpdateTimeout(context,millisToWait * NS_IN_MS);
	*/
    return 0;
}

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

    int error = ProcessStart(context,  newProcess,filename, context->ep_cap_path ,senderProcess, seL4_MaxPrio);
    
    if (error == 0)
    {
        error = !ProcessTableAppend(newProcess);
        assert(error == 0);
    }

    /**/
    free(filename);
    
    seL4_SetMR(0,__SOFA_NR_execve);
    seL4_SetMR(1, newProcess->_pid/*  -ENOSYS*/ ); // return code is the new pid
    seL4_Reply( message );
        return 0;
}
