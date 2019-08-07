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
#include "Sofa.h"
#include <SysCalls.h>
#include <SysCaps.h>
#include "KObject/KObject.h"
#include "KObject/uthash.h"


#define PROCESS_DEFAULT_CAPS SofaCap_Spawn


typedef struct
{
    SofaCapabilities caps;
} Capabilities;

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

typedef struct _Process
{
    KSet base;
	sel4utils_process_t native;
    sel4osapi_process_env_t *env;
    void *venv; // This is the shared addr of env
	uint32_t pid;
    
    ProcessState status;
    
    Capabilities caps;
	
    uint32_t timerID; // > 0 if allocated. Used to sleep
    
    seL4_CPtr reply; // slot for async replies
    ReplyState replyState;
    
    Stats stats;
    
    uint8_t priority;
    
    int retCode;
    SofaSignal retSignal;
    
    // global process list
    UT_hash_handle hh; /* makes this structure hashable */
    
    UT_hash_handle serversList; /* When the process is a server */
    //UT_hash_handle clientsList; /* When the process is a client */
    
} Process;

static inline NO_NULL_POINTERS char* ProcessGetIPCBuffer( Process *p)
{
    return p->env->mainThreadEnv.buf;
}

static inline NO_NULL_POINTERS const char* ProcessGetName( const Process* p)
{
    return p->base.obj.k_name;
}

static inline NO_NULL_POINTERS Process* ProcessGetParent( Process*p)
{
    return (Process*) p->base.obj._parent;
}

int ProcessListInit(void);
void ProcessInit(Process*p) NO_NULL_POINTERS;

size_t ProcessListGetCount(void);

size_t ProcessGetChildrenCount(Process* process) NO_NULL_POINTERS;

Process* ProcessGetFirstChild(Process* fromProcess) NO_NULL_POINTERS;
Process* ProcessGetFirstChildZombie(Process* fromProcess) NO_NULL_POINTERS;

// signal can be 0, ie no signal
int ProcessKill( Process* process , SofaSignal signal);

int ProcessCleanup( Process* process) NO_NULL_POINTERS;
Process* ProcessGetByPID( uint32_t pid);

int ProcessStart(Process *process , const char* name , vka_object_t *fromEp, Process *parent);


int ProcessSetPriority( Process* process , int prio);
int ProcessGetPriority( Process* process , int *prio) NO_NULL_POINTERS;

void ProcessDump(void);


static inline int ProcessHasCap(Process* proc , SofaCapabilities cap)
{
    return (proc->caps.caps & cap) == cap;
}
void ProcessDumpCaps(Process *proc) NO_NULL_POINTERS;
