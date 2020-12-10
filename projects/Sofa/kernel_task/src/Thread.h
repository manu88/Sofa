#pragma once
#include <sys/types.h>
#include <sel4/types.h>

typedef struct _ServiceClient ServiceClient;

typedef struct _Process Process;
/*
Shared structure beetwen kernel_task threads and process threads.
See KThread and Thread.
*/
typedef struct
{
    uint8_t kernTaskThread;
    Process* process; //process owner

    seL4_Word replyCap;
    unsigned int timerID;
    ServiceClient* clients; // a list of Service clients belonging to this thread.


} ThreadBase;
