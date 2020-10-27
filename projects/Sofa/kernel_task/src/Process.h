#pragma once

#include <utils/uthash.h>
#include <sel4utils/process.h>
#include <proc_ctx.h>

struct _Process
{
    int pid;
    sel4utils_process_t _process;
    seL4_CPtr endpoint;

    ProcessContext* ctx;

    // Hash handle for Global Proc table
    UT_hash_handle hh;

    // Hash handle for child processes
    UT_hash_handle hchld;

    struct _Process *parent;

    
    struct _Process* children;
};

typedef struct _Process Process;

void ProcessInit(Process*p);

size_t Process_GetNextPID(void);

void Process_Add(Process* p);
void Process_Remove(Process* p);
Process* Process_GetByPID(int pid);



void Process_AddChild(Process* parent, Process* child);
size_t Process_CountChildren( const Process* p);