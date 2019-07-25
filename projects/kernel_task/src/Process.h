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


#include <sel4utils/process.h>
#include <SysCalls.h>
#include "KObject/KObject.h"
#include "KObject/uthash.h"

typedef enum
{
    ReplyState_None,
    ReplyState_Sleep,
    ReplyState_Wait,
} ReplyState;

typedef enum
{
    ProcessState_Stopped,
    ProcessState_Run,
    ProcessState_Zombie,
} ProcessState;

typedef struct
{
    uint64_t numSysCalls;
    uint64_t startedTime;
} Stats;

#define MAX_PROCESS_NAME 128
typedef struct _Process
{
    KSet base;
	sel4utils_process_t native;
	uint32_t pid;
    
    ProcessState status;
	
    ThreadEnvir* env; // This is shared with the process and defined in libSysCall
    void *vaddr; // This is the shared addr of env

    uint32_t timerID; // > 0 if allocated. Used to sleep
    
    seL4_CPtr reply; // slot for async replies
    ReplyState replyState;
    
    Stats stats;
    
    int retCode;
    
    // global process list
    UT_hash_handle hh; /* makes this structure hashable */
    
    UT_hash_handle serversList; /* When the process is a server */
    //UT_hash_handle clientsList; /* When the process is a client */
    
} Process;

static inline const char* ProcessGetName( const Process* p)
{
    return p->base.obj.k_name;
}

static inline Process* ProcessGetParent( Process*p)
{
    return (Process*) p->base.obj._parent;
}

int ProcessListInit(void);
void ProcessInit(Process*p);

size_t ProcessListGetCount(void);

size_t ProcessGetChildrenCount(Process* process);

Process* ProcessGetFirstChild(Process* fromProcess);
Process* ProcessGetFirstChildZombie(Process* fromProcess);

int ProcessKill( Process* process);

int ProcessCleanup( Process* process);
Process* ProcessGetByPID( uint32_t pid);

int ProcessStart(Process *process , const char* name , vka_object_t *fromEp, Process *parent);


void ProcessDump(void);
