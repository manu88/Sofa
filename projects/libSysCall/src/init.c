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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>



static seL4_CPtr endpoint = 0;
static ThreadEnvir* env = NULL;

int InitClient(const char* EPString )
{
	endpoint = (seL4_CPtr) atoi(EPString/*argv[argc-1]*/);

    seL4_Word badge;
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    //seL4_Recv(endpoint, &badge);
    seL4_SetMR(0 , SysCall_BootStrap);
    
    
    seL4_Call(endpoint, info);
    
    assert(seL4_MessageInfo_get_label(info) == seL4_Fault_NullFault);
    assert(seL4_MessageInfo_get_length(info) == 2);
    assert(seL4_GetMR(0) == SysCall_BootStrap);
    env = (ThreadEnvir*)seL4_GetMR(1);

	return 0;
}

void StopClient(int retCode)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Exit);
    seL4_SetMR(1 , retCode);
    
    seL4_Send(endpoint, info);
}


void print(const char *format, ...)
{
	assert(env);
	
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
	seL4_SetMR(0 , SysCall_Write);
	// 2nd reg is for return value;

	va_list vl;
    va_start(vl, format);

    vsnprintf( env->buf, IPC_BUF_SIZE, format, vl);

    va_end( vl);

	seL4_Call(endpoint, info);

	assert(seL4_GetMR(0) == SysCall_Write);
}


int spawn(const char* name,int argc , char* argv[])
{
	assert(env);
        
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Spawn);
    // 2nd reg is for return value;

	snprintf( env->buf, IPC_BUF_SIZE, "%s" , name);

    seL4_Call(endpoint, info);

    assert(seL4_GetMR(0) == SysCall_Spawn);
	return (int) seL4_GetMR(1);
}

int kill(int pid)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Kill);
    seL4_SetMR(1 , pid);
    // 2nd reg is for return value;
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_Kill);
    return (int) seL4_GetMR(1);
}

long wait(int *wstatus)
{
    if( wstatus == NULL)
        return -1;
    
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_Wait);
    // 2nd reg is for returned pid;
    // 3rd reg is for returned status;
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_Wait);
    *wstatus = (int) seL4_GetMR(2);
    
    return (long) seL4_GetMR(1);
}

long unsigned int getTime()
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_GetTime);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_GetTime);
    
    return (int) seL4_GetMR(1);
}

static int _Sleep( unsigned long l , SleepUnit unit)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_Sleep);
    seL4_SetMR(1 , l);
    seL4_SetMR(2 , unit);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_Sleep);
    
    return (int) seL4_GetMR(1);
}
int sleepS( unsigned long sec)
{
    return _Sleep(sec , SleepUnit_S);
}
int sleep( unsigned long ns)
{
    return _Sleep(ns , SleepUnit_NS);
}

int sleepMS( unsigned long ms)
{
    return _Sleep(ms , SleepUnit_MS);
}

long read(char*buf, unsigned long count)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Read);
    seL4_SetMR(1 , count);
    
    
    seL4_Call(endpoint, info);

    assert(seL4_GetMR(0) == SysCall_Read);
    memcpy(buf ,env->buf, seL4_GetMR(1) );
    return (long) seL4_GetMR(1);
}

static void doDebugSysCall(SysCall_Debug_ID msgID)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    
    seL4_SetMR(0 , SysCall_Debug);
    seL4_SetMR(1 , msgID);
    
    seL4_Send(endpoint, info);
}

void ps()
{
    doDebugSysCall(SysCall_Debug_PS);
}

void sched()
{
    doDebugSysCall(SysCall_Debug_Sched);
}

void listServers()
{
    doDebugSysCall(SysCall_Debug_ListServers);
}


ServerEnvir* RegisterServerWithName(const char*name, int flags)
{
    if( !name )
        return NULL;
    
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_RegisterServer);
    seL4_SetMR(1 , flags);
    // 2nd reg is for return value;
    // 3rd will get the ServerEnv pointer
    
    snprintf( env->buf, IPC_BUF_SIZE, "%s" , name);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_RegisterServer);
    
    if( seL4_GetMR(1) == 0)
    {
        return (ServerEnvir*) seL4_GetMR(2);
    }
    
    return NULL;
}

ClientEnvir* ConnectToServer( const char*name)
{
    if( !name )
        return NULL;
    
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_RegisterClient);
    // 2nd reg is for return value;
    // 3rd will get the ServerEnv pointer
    
    snprintf( env->buf, IPC_BUF_SIZE, "%s" , name);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_RegisterClient);
    
    if( seL4_GetMR(1) == 0)
    {
        return (ClientEnvir*) seL4_GetMR(2);
    }
    
    return NULL;
}

int ServerRecv(ServerEnvir* server)
{
    seL4_Word sender_badge = 0;
    seL4_MessageInfo_t message = seL4_Recv(server->endpoint, &sender_badge);
    
    return (int)sender_badge;
}


