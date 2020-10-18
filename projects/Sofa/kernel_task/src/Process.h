#pragma once

#include <sel4utils/process.h>

typedef struct
{
    sel4utils_process_t _process;
    seL4_CPtr endpoint;
} Process;
