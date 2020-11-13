#include "SyscallTable.h"
#include <Sofa.h>
#include <vka/capops.h>

void Syscall_ThreadNew(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    assert(process);
    printf("Received thead ep request from '%s' %i\n", ProcessGetName(process), ProcessGetPID(process));

    Thread* newThread = malloc(sizeof(Thread));
    assert(newThread);
    memset(newThread, 0, sizeof(Thread));
    newThread->process = process;

    cspacepath_t badged_ep_path;
    int error = vka_cspace_alloc_path(&env->vka, &badged_ep_path);
    ZF_LOGF_IFERR(error, "Failed to allocate path\n");
    assert(error == 0);
    cspacepath_t ep_path = {0};
    vka_cspace_make_path(&env->vka, env->root_task_endpoint.cptr, &ep_path);

    error = vka_cnode_mint(&badged_ep_path, &ep_path, seL4_AllRights, (seL4_Word)newThread);
    newThread->process_endpoint = badged_ep_path.capPtr;
    assert(error == 0);

    seL4_MessageInfo_t infoRet = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 1, 1);
    seL4_SetMR(0, SyscallID_ThreadNew);
    LL_APPEND(process->threads, newThread);
    seL4_SetCap(0, badged_ep_path.capPtr);
    seL4_Reply(infoRet);
}


void Syscall_ThreadExit(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    assert(process);
    LL_DELETE(process->threads, caller);
    free(caller);
}