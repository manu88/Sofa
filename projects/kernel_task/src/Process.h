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
//    ReplyState_Wait,
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
    //char name[MAX_PROCESS_NAME];
	
    ThreadEnvir* env; // This is shared with the process and defined in libSysCall
    void *vaddr; // This is the shared addr of env

    uint32_t timerID; // > 0 if allocated. Used to sleep
    
    seL4_CPtr reply; // slot for async replies
    ReplyState replyState;
    
    struct _Process *parent;

    Stats stats;
    
    
    int retCode;
    
    // global process list
    struct _Process *prev,*next;
    
    // global process list
    UT_hash_handle hh; /* makes this structure hashable */
    
} Process;

static inline const char* ProcessGetName( const Process* p)
{
    return p->base.obj.k_name;
}

int ProcessListInit(void);
void ProcessInit(Process*p);

size_t ProcessListGetCount(void);

size_t ProcessGetChildrenCount(Process* process);

Process* ProcessGetFirstChild(Process* fromProcess);

int ProcessKill( Process* process);

int ProcessCleanup( Process* process);
Process* ProcessGetByPID( uint32_t pid);

int ProcessStart(Process *process , const char* name , vka_object_t *fromEp, Process *parent);


void ProcessDump(void);
