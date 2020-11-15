#include "SyscallTable.h"
#include "../testtypes.h"

void Syscall_spawn(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;

    const char* dataBuf = caller->ipcBuffer;
    printf("Spawn request from %i '%s' -> '%s'\n", ProcessGetPID(process), ProcessGetName(process), dataBuf);

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    spawnApp(env, newProc, dataBuf);
    seL4_DebugDumpScheduler();
    seL4_SetMR(1, newProc->init->pid);
    seL4_Reply(info);
}


void Syscall_wait(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;


    pid_t pidToWait = seL4_GetMR(1);
    int options = seL4_GetMR(2);
    printf("Wait request from %i '%s' on %i\n", ProcessGetPID(process), ProcessGetName(process), pidToWait);

}