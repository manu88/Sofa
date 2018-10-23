#pragma once



#include <sel4utils/process.h>



struct _Process
{
    sel4utils_process_t _process;
    pid_t               _pid;

    struct _Process *_parent;
};

typedef struct _Process Process;


int ProcessInit(Process* process);
Process* ProcessAlloc(void);
int ProcessRelease(Process* process);
