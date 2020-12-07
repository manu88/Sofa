#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"
#include "VFS.h"


void Syscall_VFS(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->_base.process;
    KernelTaskContext* env = getKernelTaskContext();


    VFSLs(caller->ipcBuffer);

}