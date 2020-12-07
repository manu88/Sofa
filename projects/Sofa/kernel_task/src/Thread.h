#pragma once
#include <sys/types.h>

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

} ThreadBase;
