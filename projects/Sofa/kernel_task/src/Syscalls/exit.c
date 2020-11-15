#include <Sofa.h>
#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"
#include "../Panic.h"


static driver_env_t *_env = NULL;

static void replyToWait(Thread* onThread, int pid, int retCode)
{
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
    seL4_SetMR(0, SyscallID_Wait);
    seL4_SetMR(1, pid);
    seL4_SetMR(2, retCode);
    seL4_Send(onThread->replyCap, tag);
    cnode_delete(&_env->vka, onThread->replyCap);

}
void Syscall_exit(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    if(!_env)
    {
        _env = env;
    }

    Process* process = caller->process;
    int retCode = seL4_GetMR(1);

    printf("[Syscall_exit] Process %i did exit with code %i\n", ProcessGetPID(process), retCode);

    Process* parent = process->parent;
    if(ProcessGetPID(process) == 1)
    {
        Panic("init returned");
    }

    Thread* waitingThread = ProcessGetWaitingThread(parent);
    if(waitingThread)
    {
        printf("[Syscall_exit] Parent %i is waiting on %i on thread %p\n",
               ProcessGetPID(parent),
               ProcessGetPID(process),
               waitingThread == &parent->main? 0: waitingThread 
              );

        assert(waitingThread->replyCap != 0);
        replyToWait(waitingThread, process->init->pid, retCode);

    }
    int shouldFree = process->init->pid > 1;
    cleanAndRemoveProcess(env, process, retCode);
    if(shouldFree)
    {
        kfree(process);
    }
}