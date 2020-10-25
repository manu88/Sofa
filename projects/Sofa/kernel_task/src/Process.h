#pragma once

#include <sel4utils/process.h>
#include<proc_ctx.h>

typedef struct
{
    sel4utils_process_t _process;
    seL4_CPtr endpoint;

    ProcessContext* ctx;
} Process;
