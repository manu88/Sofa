#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"


void Syscall_spawn(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* process = caller->process;

    const char* dataBuf = caller->ipcBuffer;

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    int ret = spawnApp(newProc, dataBuf, process);
    printf("After spawn %zi pages\n", getNumPagesAlloc());

    if(ret != 0)
    { 
        printf("[Syscall_spawn] spawn returned %i\n",ret);
        kfree(newProc);
        seL4_SetMR(1, ret);
    }
    else
    {
        seL4_SetMR(1, newProc->init->pid);
    }
    seL4_Reply(info);
}   