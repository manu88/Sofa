#include "SyscallTable.h"
#include <Sofa.h>
#include <vka/capops.h>

void Syscall_ThreadNew(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    assert(process);

    Thread* newThread = kmalloc(sizeof(Thread));
    assert(newThread);
    memset(newThread, 0, sizeof(Thread));
    newThread->process = process;
// create per thread endpoint
    cspacepath_t badged_ep_path;
    int error = vka_cspace_alloc_path(&env->vka, &badged_ep_path);
    ZF_LOGF_IFERR(error, "Failed to allocate path\n");
    assert(error == 0);
    cspacepath_t ep_path = {0};
    vka_cspace_make_path(&env->vka, env->root_task_endpoint.cptr, &ep_path);

    error = vka_cnode_mint(&badged_ep_path, &ep_path, seL4_AllRights, (seL4_Word)newThread);
    newThread->process_endpoint = badged_ep_path.capPtr;
    assert(error == 0);
// create per thread IPC page
    newThread->ipcBuffer = (uint8_t*) vspace_new_pages(&env->vspace, seL4_AllRights, 1, PAGE_BITS_4K);
    assert(newThread->ipcBuffer);
    newThread->ipcBuffer_vaddr = vspace_share_mem(&env->vspace, &process->native.vspace, newThread->ipcBuffer, 1, PAGE_BITS_4K, seL4_ReadWrite, 1);
    assert(newThread->ipcBuffer_vaddr);


// reply
    seL4_MessageInfo_t infoRet = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 1, 2);
    seL4_SetMR(0, SyscallID_ThreadNew);
    seL4_SetMR(1,(seL4_Word) newThread->ipcBuffer_vaddr);
    LL_APPEND(process->threads, newThread);
    seL4_SetCap(0, badged_ep_path.capPtr);
    seL4_Reply(infoRet);
}


void Syscall_ThreadExit(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    assert(process);
    LL_DELETE(process->threads, caller);
    kfree(caller);
}