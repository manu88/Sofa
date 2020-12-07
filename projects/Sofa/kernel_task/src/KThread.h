#pragma once
#include <sel4utils/thread.h>
#include "Thread.h"

typedef struct _KThread KThread;

typedef void (*KThreadMain)(KThread* thread, void *arg);

typedef struct _KThread
{
    ThreadBase _base; // needs to remain fisrt!!
    seL4_CPtr ep;
    sel4utils_thread_t native;
    KThreadMain mainFunction;
    const char* name; // can be NULL

}KThread;

void KThreadInit(KThread* t);

int KThreadRun(KThread* t, int prio, void* arg);