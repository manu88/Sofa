#pragma once

#include <utils/uthash.h>
#include <sel4utils/process.h>
#include <proc_ctx.h>

typedef struct
{
    int pid;
    sel4utils_process_t _process;
    seL4_CPtr endpoint;

    ProcessContext* ctx;

    UT_hash_handle hh;
} Process;


void ProcessInit(Process*p);

size_t Process_GetNextPID(void);

void Process_Add(Process* p);
void Process_Remove(Process* p);
Process* Process_GetByPID(int pid);