#pragma once



#include <sel4utils/process.h>
#include "TimerWheel/TimersWheel.h"
#include "Bootstrap.h"


struct _Process
{
    sel4utils_process_t _process;
    pid_t               _pid;


    struct _Process *_parent;

    seL4_CPtr reply;
    //Timer* _timer;
};

typedef struct _Process Process;


int ProcessInit(Process* process);
Process* ProcessAlloc(void);
int ProcessRelease(Process* process);


int startProcess(InitContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority );
