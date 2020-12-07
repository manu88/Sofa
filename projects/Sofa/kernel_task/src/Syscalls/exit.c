#include <Sofa.h>
#include "SyscallTable.h"
#include "../testtypes.h"
#include "KThread.h"


void Syscall_exit(Thread* caller, seL4_MessageInfo_t info)
{
    int retCode = seL4_GetMR(1);

    ThreadBase* base = (ThreadBase*) caller;
    if(base->kernTaskThread)
    {
        printf("Exit from kernel_task thread with status %i\n", retCode);
        KThreadCleanup((KThread*) caller);
        return;
    }
    KernelTaskContext* env = getKernelTaskContext();

    Process* process = caller->process;
    doExit(process, retCode);
}