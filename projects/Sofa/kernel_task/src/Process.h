#pragma once

#include <sel4utils/process.h>


typedef struct
{
    void *remote_vaddr; // the shared mem address for the process to retreive its init stuff
    sel4utils_process_t native;
    seL4_CPtr process_endpoint;
} Process;
