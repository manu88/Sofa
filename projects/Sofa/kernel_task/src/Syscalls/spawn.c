#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"


void Syscall_spawn(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* process = caller->process;

    const char* dataBuf = caller->ipcBuffer;
    printf("Spawn request from %i '%s' -> '%s'\n", ProcessGetPID(process), ProcessGetName(process), dataBuf);

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    spawnApp(newProc, dataBuf, process);
    seL4_DebugDumpScheduler();
    seL4_SetMR(1, newProc->init->pid);
    seL4_Reply(info);
}