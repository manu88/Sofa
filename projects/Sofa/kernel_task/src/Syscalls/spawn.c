#include "SyscallTable.h"
#include "../testtypes.h"

void Syscall_spawn(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;

    const char* dataBuf = caller->ipcBuffer;
    printf("Spawn request from %i '%s' -> '%s'\n", ProcessGetPID(process), ProcessGetName(process), dataBuf);

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    spawnApp(env, newProc, "app");
    seL4_DebugDumpScheduler();
    seL4_SetMR(1, newProc->init->pid);
    seL4_Reply(info);

}