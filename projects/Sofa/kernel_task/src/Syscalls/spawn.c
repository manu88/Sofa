#include "SyscallTable.h"
#include "../testtypes.h"

void Syscall_spawn(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    printf("Spawn request from %i '%s'\n", ProcessGetPID(process), ProcessGetName(process));

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    spawnApp(env, newProc, "app");
    seL4_DebugDumpScheduler();
    seL4_SetMR(1, newProc->init->pid);
    seL4_Reply(info);

}