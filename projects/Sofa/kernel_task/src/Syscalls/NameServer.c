#include <Sofa.h>
#include "SyscallTable.h"


void Syscall_RegisterService(Thread* caller, seL4_MessageInfo_t info)
{
    printf("Register Service name %s", caller->ipcBuffer);

    seL4_SetMR(1, 0);
    seL4_Reply(info);

}