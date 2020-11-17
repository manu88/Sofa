#include <Sofa.h>
#include "SyscallTable.h"
#include "../testtypes.h"



void Syscall_exit(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();

    Process* process = caller->process;
    int retCode = seL4_GetMR(1);
    doExit(process, retCode);
}