#include <Sofa.h>
#include "SyscallTable.h"

void Syscall_PPID(Thread* caller, seL4_MessageInfo_t info)
{
    int ppid = ProcessGetPID(caller->process->parent);

    seL4_SetMR(1, ppid);
    seL4_Reply(info);
}