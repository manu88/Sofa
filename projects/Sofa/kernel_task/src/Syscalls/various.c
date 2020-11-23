#include <Sofa.h>
#include <sel4utils/thread.h>   
#include "SyscallTable.h"

void Syscall_PPID(Thread* caller, seL4_MessageInfo_t info)
{
    int ppid = ProcessGetPID(caller->process->parent);

    seL4_SetMR(1, ppid);
    seL4_Reply(info);
}
//SofaRequestCap_VSpaceRoot vka_alloc_vspace_root



void RequestCapTCB(Thread* caller, seL4_MessageInfo_t info)
{
    vka_object_t tcb_obj;
    vka_alloc_tcb(&getKernelTaskContext()->vka, &tcb_obj);
    cspacepath_t res;
    vka_cspace_make_path(&getKernelTaskContext()->vka, tcb_obj.cptr, &res);
    seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->process->native, res, &getKernelTaskContext()->vka);
    seL4_SetMR(1, ret);
    seL4_Reply(info);
}

void RequestMap(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->process;
    KernelTaskContext* env = getKernelTaskContext();
    void* p = vspace_new_sized_stack(&caller->process->native.vspace, 1);

    Thread* newThread = kmalloc(sizeof(Thread));
    assert(newThread);
    memset(newThread, 0, sizeof(Thread));
    newThread->process = process;
    LL_APPEND(process->threads, newThread);
    newThread->stack = p;
    newThread->stackSize = 1;

    //  vspace_new_pages(&env->vspace, seL4_AllRights, 1, PAGE_BITS_4K);
    assert(p);

    void* p_addr = p;// vspace_share_mem(&env->vspace, &caller->process->native.vspace, p, 1, PAGE_BITS_4K,seL4_AllRights, 1);
    assert(p_addr);
    seL4_SetMR(1, p_addr);
    seL4_Reply(info);
}

void Syscall_RequestCap(Thread* caller, seL4_MessageInfo_t info)
{
    printf("[Syscall_RequestCap] got a request\n");

    const SofaRequestCap op = seL4_GetMR(1);
    switch (op)
    {
    case SofaRequestCap_TCB:
        RequestCapTCB(caller, info);
        break;    
    case SofaRequestCap_MAP:
        RequestMap(caller, info);
        break;

    default:
        assert(0);
        break;
    }
}