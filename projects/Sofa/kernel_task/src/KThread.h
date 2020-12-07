#pragma once
#include <sel4utils/thread.h>
#include "Thread.h"

typedef struct _KThread KThread;

typedef int (*KThreadMain)(KThread* thread, void *arg);

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
void KThreadCleanup(KThread* t);


int KThreadSleep(KThread* t, int ms);
void KThreadExit(KThread* t, int code);

// Works only in threads, so be sure not to call it in main Thread!
int KSleep(int ms);
