#pragma once
#include <sys/types.h>

/*
Shared structure beetwen kernel_task threads and process threads.
See KThread and Thread.
*/
typedef struct
{
    uint8_t kernTaskThread;
    seL4_Word replyCap;
    unsigned int timerID;

} ThreadBase;
