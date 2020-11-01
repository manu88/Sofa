#pragma once

#include <utils/uthash.h>
#include <sel4utils/process.h>
#include <proc_ctx.h>
#include "ThreadContext.h"


typedef enum
{
    ProcessState_Stopped,
    ProcessState_Run,
    ProcessState_Zombie

} ProcessState;


struct _Process
{
    char* name;
    int pid;
    sel4utils_process_t _process;
//    seL4_CPtr endpoint;

    ProcessContext* ctx;
    ThreadContext main;

    // Hash handle for Global Proc table
    UT_hash_handle hh;

    // Hash handle for child processes
    UT_hash_handle hchld;

    struct _Process *parent;

    // This is a Hash map
    struct _Process* children;

    ProcessState state;

    uint8_t _isWaiting;
};

typedef struct _Process Process;

Process* ProcessGetList(void);

#define ProcessListIter(p, tmp) HASH_ITER(hh, ProcessGetList(), p, tmp)

void ProcessInit(Process*p);

size_t Process_GetNextPID(void);

void Process_Add(Process* p);
void Process_Remove(Process* p);
Process* Process_GetByPID(int pid);


int Process_IsWaiting(const Process* p);
void Process_AddChild(Process* parent, Process* child);
void Process_RemoveChild(Process* parent, Process* child);
size_t Process_CountChildren( const Process* p);