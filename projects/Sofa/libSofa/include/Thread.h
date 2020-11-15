#pragma once
#include <helpers.h>

typedef struct
{
    helper_thread_t th; // Needs to remain 1st!
    seL4_CPtr ep;

    void* ret;
} Thread;

typedef void *(*start_routine) (void *);

int ThreadInit(Thread* t, start_routine threadMain, void* arg);

int ThreadJoin(Thread* t, void **retval);