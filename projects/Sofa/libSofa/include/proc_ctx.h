#pragma once

#include <utils/compile_time.h>
#include <utils/page.h>

typedef struct
{

    int test;
} ProcessContext;


compile_time_assert(ProcessContext_fits_in_ipc_buffer, sizeof(ProcessContext) < PAGE_SIZE_4K);