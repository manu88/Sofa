/*
 * This file is part of the Sofa project
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

#include "sel4process.h"
#include "utlist.h"
#include "test_init_data.h"
#include "Environ.h"
#include "Allocator.h"
#include "Thread.h"



typedef enum
{
    ProcessState_Running, // This is not 'running' in the sense 'NOT sleeping' but running is the sense 'valid'
    ProcessState_Zombie,
    ProcessState_Stopped
} ProcessState;

typedef struct _Process Process;

typedef struct _Thread
{
    ThreadBase _base; // needs to remain fisrt!!
    seL4_CPtr process_endpoint;

    uint8_t* ipcBuffer_vaddr;
    uint8_t* ipcBuffer;
//    seL4_Word replyCap;

    struct _Thread *next;
    void *stack;
    size_t stackSize; // in pages

    
    vka_object_t tcb; // the tcb cap, kernel_task' side
    void* ipcBuf2;

} Thread;

typedef struct _ProcStats
{
    uint64_t allocPages;
    uint64_t startTime;
}ProcStats;

typedef struct _Process
{
    sel4utils_process_t native;

    Thread main;
    
    const char* name; // pointer to init->name, or static string in init's case.

    char **argv; // program arguments
    int argc; // program args count

    void *init_remote_vaddr; // the shared mem address for the process to retreive its init stuff
    test_init_data_t *init; // init stuff. valid on kernel_task' side, for process side, use 'init_remote_vaddr'


    Thread* threads; // other threads, NOT including the main one
 
    int retCode; // combination of retcode + signal. see man 2 wait for specs. and the macro MAKE_EXIT_CODE somewhere in this source code.
    ProcessState state;

    struct _Process *parent;
    struct _Process* children;

    struct _Process* next; // For Global process list
    struct _Process* nextChild; // For Children

    ProcStats stats;
} Process;


// Process methods

static inline void ProcessInit(Process* p)
{
    memset(p, 0, sizeof(Process));
    p->main._base.process = p;
}

static inline const char* ProcessGetName(const Process* p)
{
    return p->name;
}

static inline int ProcessGetPID(const Process* p)
{
    if(p == getKernelTaskProcess())
    {
        return 0;
    }
    return p->init->pid;
}

int ProcessCountExtraThreads(const Process* p);

Thread* ProcessGetWaitingThread(Process*p);

#define PROCESS_FOR_EACH_EXTRA_THREAD(proc, t) LL_FOREACH(proc->threads,t)

void ProcessAddChild(Process* parent, Process* child);
void ProcessRemoveChild(Process* parent, Process* child);
int ProcessCoundChildren(const Process* p);

static inline Process* ProcessGetChildren(Process* p)
{
    return p->children;
}

#define PROCESS_FOR_EACH_CHILD(proc, c) LL_FOREACH(proc->children,c)



int ProcessSuspend(Process* p);
int ProcessResume(Process* p);

// Thread methods

void ThreadCleanupTimer(Thread* t);
static inline uint8_t ThreadIsWaiting(const Thread* t)
{
    return t->_base.replyCap != 0;
}

// Process List methods

int ProcessListInit(void);
Process* getProcessList(void);

// guarded by the process list mutex
void ProcessListAdd(Process* p);

// guarded by the process list mutex
void ProcessListRemove(Process* p);

// guarded by the process list mutex
size_t ProcessListCount(void);

// guarded by the process list mutex
Process* ProcessListGetByPid(pid_t pid);


int ProcessListLock(void);
int ProcessListUnlock(void);

// caller needs to call ProcessListLock/ ProcessListUnlock !
#define FOR_EACH_PROCESS(p) LL_FOREACH(getProcessList(),p)

