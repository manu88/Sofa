#pragma once

#include <sel4utils/process.h>
#include "utlist.h"
#include "test_init_data.h"
#include "Environ.h"
#include "Allocator.h"




typedef enum
{
    ThreadState_Running = 0,
    ThreadState_Sleeping,
    ThreadState_Waiting,
} ThreadState;

typedef enum
{
    ProcessState_Running,
    ProcessState_Exit
} ProcessState;

typedef struct _Process Process;


typedef struct _Thread
{
    seL4_CPtr process_endpoint;

    uint8_t* ipcBuffer_vaddr;
    uint8_t* ipcBuffer;
    seL4_Word replyCap;
    Process* process;

    unsigned int timerID;

    ThreadState state;
    struct _Thread *next; 
} Thread;

typedef struct _Process
{
    Thread main;
    sel4utils_process_t native;
    
    void *init_remote_vaddr; // the shared mem address for the process to retreive its init stuff
    test_init_data_t *init; // init stuff. valid on kernel_task' side, for process side, use 'init_remote_vaddr'
    UntypedRange untypedRange;

    struct _Process *parent;

    Thread* threads; // other threads, NOT including the main one
    ProcessState state;

    struct _Process* children;
    struct _Process* next; // For Global process list
    struct _Process* nextChild; // For Children
} Process;


// Process methods

static inline void ProcessInit(Process* p)
{
    memset(p, 0, sizeof(Process));
    p->main.process = p;
}

static inline const char* ProcessGetName(const Process* p)
{
    return p->init->name;
}

static inline int ProcessGetPID(const Process* p)
{
    return p->init->pid;
}

int ProcessCountExtraThreads(const Process* p);


Thread* ProcessGetWaitingThread( const Process*p);

#define PROCESS_FOR_EACH_EXTRA_THREAD(proc, t) LL_FOREACH(proc->threads,t)



void ProcessAddChild(Process* parent, Process* child);
void ProcessRemoveChild(Process* parent, Process* child);
int ProcessCoundChildren(const Process* p);
// Thread methods

void ThreadCleanupTimer(Thread* t);

// Process List methods
Process* getProcessList(void);
void ProcessListAdd(Process* p);
void ProcessListRemove(Process* p);

#define FOR_EACH_PROCESS(p) LL_FOREACH(getProcessList(),p)