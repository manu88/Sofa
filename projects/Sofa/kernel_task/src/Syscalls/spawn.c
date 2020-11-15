#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"


void Syscall_spawn(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;

    const char* dataBuf = caller->ipcBuffer;
    printf("Spawn request from %i '%s' -> '%s'\n", ProcessGetPID(process), ProcessGetName(process), dataBuf);

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    spawnApp(env, newProc, dataBuf, process);
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



    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        printf("[Syscall_wait]Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }
    caller->replyCap = slot;
    caller->state = ThreadState_Waiting;
    printf("[Syscall_wait] Go into wait state\n");

}