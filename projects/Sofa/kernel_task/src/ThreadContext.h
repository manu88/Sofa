#pragma once

#include <sel4/sel4.h>
#include <utils/uthash.h>

typedef enum
{
    ThreadState_None,
    ThreadState_Sleep,
    ThreadState_Wait,
} ThreadState;

typedef struct
{
    seL4_CPtr endpoint;
    ThreadState state;

    // Hash handle for Global waiting list
    UT_hash_handle hh;

    seL4_CPtr reply; // slot for async replies
} ThreadContext;


void WaitList_Add(ThreadContext* context);

