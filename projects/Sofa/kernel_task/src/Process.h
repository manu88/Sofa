#pragma once

#include <sel4utils/process.h>


typedef struct
{
    sel4utils_process_t native;
    seL4_CPtr process_endpoint;
    void *init_remote_vaddr; // the shared mem address for the process to retreive its init stuff

    test_init_data_t *init; // init stuff. valid on kernel_task' side, for process side, use 'init_remote_vaddr'
} Process;
