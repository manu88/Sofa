#pragma once

#include <sel4utils/process.h>
#include "utlist.h"
#include "test_init_data.h"
#include "test.h"


typedef struct _Process Process;

typedef struct _Thread
{
    seL4_CPtr process_endpoint;
    seL4_Word replyCap;
    Process* process;

    unsigned int timerID;

    struct _Thread *next; 
} Thread;

typedef struct _Process
{
    Thread main;
    sel4utils_process_t native;
    
    void *init_remote_vaddr; // the shared mem address for the process to retreive its init stuff

    test_init_data_t *init; // init stuff. valid on kernel_task' side, for process side, use 'init_remote_vaddr'

    int untyped_index_start;
    int untyped_index_size;

    Thread* threads; // other threads, NOT including the main one

    struct _Process* next; // Global process list
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

// Thread methods

void ThreadCleanupTimer(Thread* t, driver_env_t *env);

// Process List methods
Process* getProcessList(void);
void ProcessListAdd(Process* p);
void ProcessListRemove(Process* p);

#define FOR_EACH_PROCESS(p) LL_FOREACH(getProcessList(),p)