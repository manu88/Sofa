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

#pragma once
#include <sel4/sel4.h>
typedef enum
{
    SysCall_BootStrap = 1, // 1st msg sent to a process by kernel_task so it can retreive its environment.
	SysCall_Write,
    SysCall_Sleep,
	SysCall_Read,
	SysCall_Debug,
	SysCall_Spawn,
    SysCall_Kill,
    SysCall_Wait,
    SysCall_Exit,
    SysCall_GetTime,
    
    SysCall_SetPriority,
    SysCall_GetPriority,
    
    SysCall_RegisterServer,
    SysCall_RegisterClient,
} SysCallID;

typedef enum
{
    SysCall_Debug_PS =1,
    SysCall_Debug_Sched,
    SysCall_Debug_ListServers,
}SysCall_Debug_ID;

typedef enum
{
    SleepUnit_NS,
    SleepUnit_MS,
    SleepUnit_S,
    
} SleepUnit;


#define IPC_BUF_SIZE 256
typedef struct
{
	char buf[IPC_BUF_SIZE];
	unsigned long bufSize;

} ThreadEnvir;



typedef struct
{
    char buf[IPC_BUF_SIZE];
    unsigned long bufSize;
    
    seL4_CPtr endpoint;
} ServerEnvir;

typedef struct
{
    char buf[IPC_BUF_SIZE];
    unsigned long bufSize;
    
    seL4_CPtr endpoint;
} ClientEnvir;

#ifndef KERNEL_TASK

int InitClient(const char* EPString );
void StopClient(int retCode);

void print(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

int spawn(const char* name, int argc , char* argv[]);

long read(char*buf, unsigned long count);

int sleep( unsigned long ns);
int sleepS( unsigned long sec);
int sleepMS( unsigned long ms);
int kill(int pid);

long wait(int *wstatus);

long unsigned int getTime(void);

void ps(void);
void sched(void);

int getPriority(int pid , int *retVal);
int setPriority(int pid , int prio);


// Server
void listServers(void);
ServerEnvir* RegisterServerWithName(const char*name, int flags);


ClientEnvir* ConnectToServer( const char*name);
int ServerRecv(ServerEnvir* server);

#endif
