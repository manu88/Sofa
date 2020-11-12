#pragma once

#include <sel4utils/process.h>
#include <utils/uthash.h>
#include "test_init_data.h"

typedef struct _Process Process;

typedef struct
{
    seL4_CPtr process_endpoint;
    Process* parent;
} Thread;

typedef struct _Process
{
    Thread main;
    sel4utils_process_t native;
    
    void *init_remote_vaddr; // the shared mem address for the process to retreive its init stuff

    test_init_data_t *init; // init stuff. valid on kernel_task' side, for process side, use 'init_remote_vaddr'

    int untyped_index_start;
    int untyped_index_size;
} Process;


static inline void ProcessInit(Process* p)
{
    memset(p, 0, sizeof(Process));
    p->main.parent = p;
}

static inline const char* ProcessGetName(const Process* p)
{
    return p->init->name;
}

static inline int ProcessGetPID(const Process* p)
{
    return p->init->pid;
}