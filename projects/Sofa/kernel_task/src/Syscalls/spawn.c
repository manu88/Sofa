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


void Syscall_Kill(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* process = caller->process;

    pid_t pidToKill = seL4_GetMR(1);

    int signal = seL4_GetMR(2);

    int ret = 0;

    printf("Kill request from %i to %i with signal %i\n", ProcessGetPID(process), pidToKill, signal);

    Process* processToKill = ProcessListGetByPid(pidToKill);
    if(processToKill == NULL)
    {
        ret = -ESRCH;
    }
    else
    {
        pid_t callerPid = ProcessGetPID(process);        
        cleanAndRemoveProcess(processToKill, -1);
        ret = 0;

        // did we just killed yourselves?
        if(pidToKill == callerPid)
        {
            // dont bother send a reply, we're dead!
            return;
        }
    }

    seL4_SetMR(1, ret);
    seL4_Reply(info);
}


void Syscall_wait(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    KernelTaskContext* env = getKernelTaskContext();

    pid_t pidToWait = seL4_GetMR(1);
    int options = seL4_GetMR(2);
    printf("Wait request from %i '%s' on %i\n", ProcessGetPID(process), ProcessGetName(process), pidToWait);

    // does the process have any children?
    if(ProcessCoundChildren(process) == 0)
    {
        printf("[Syscall_wait] no children, return\n");
        seL4_SetMR(1, -1);
        seL4_SetMR(2, 0); // wstatus
        seL4_Reply(info);
        return;
    }


    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        printf("[Syscall_wait] Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }
    caller->replyCap = slot;
    caller->state = ThreadState_Waiting;
    printf("[Syscall_wait] Go into wait state\n");

}