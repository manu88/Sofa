#include <Sofa.h>
#include <sel4utils/thread.h>   
#include "SyscallTable.h"

void Syscall_PPID(Thread* caller, seL4_MessageInfo_t info)
{
    int ppid = ProcessGetPID(caller->_base.process->parent);

    seL4_SetMR(1, ppid);
    seL4_Reply(info);
}

void RequestCapEndoint(Thread* caller, seL4_MessageInfo_t info)
{
    vka_object_t endpoint_obj;
    vka_alloc_endpoint(&getKernelTaskContext()->vka, &endpoint_obj);
    cspacepath_t res;
    vka_cspace_make_path(&getKernelTaskContext()->vka, endpoint_obj.cptr, &res);
    seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &getKernelTaskContext()->vka);
    seL4_SetMR(1, ret);
    seL4_Reply(info);
}

void RequestCapTCB(Thread* caller, seL4_MessageInfo_t info)
{
    vka_object_t tcb_obj;
    vka_alloc_tcb(&getKernelTaskContext()->vka, &tcb_obj);
    cspacepath_t res;
    vka_cspace_make_path(&getKernelTaskContext()->vka, tcb_obj.cptr, &res);
    seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &getKernelTaskContext()->vka);
    seL4_SetMR(1, ret);
    seL4_Reply(info);
}

void RequestIPCBuff(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->_base.process;
    KernelTaskContext* env = getKernelTaskContext();
    seL4_CPtr *buf;
    void* bufAddr = vspace_new_ipc_buffer(&caller->_base.process->native.vspace, buf);

    seL4_SetMR(1, (seL4_Word) bufAddr);
    seL4_Reply(info);

}

void RequestNewThread(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->_base.process;
    KernelTaskContext* env = getKernelTaskContext();
    void* p = vspace_new_sized_stack(&caller->_base.process->native.vspace, 1);

    Thread* newThread = kmalloc(sizeof(Thread));
    assert(newThread);
    memset(newThread, 0, sizeof(Thread));
    newThread->_base.process = process;
    LL_APPEND(process->threads, newThread);
    newThread->stack = p;
    newThread->stackSize = 1;

    assert(p);

    seL4_SetMR(1, (seL4_Word) p);
    seL4_Reply(info);
}

void RequestCapNewPage(Thread* caller, seL4_MessageInfo_t info)
{

}


void Syscall_RequestCap(Thread* caller, seL4_MessageInfo_t info)
{
    KLOG_INFO("[Syscall_RequestCap] got a request\n");

    const SofaRequestCap op = seL4_GetMR(1);
    switch (op)
    {
    case SofaRequestCap_TCB:
        RequestCapTCB(caller, info);
        break;    
    case SofaRequestCap_NewThread:
        RequestNewThread(caller, info);
        break;
    case SofaRequestCap_IPCBuff:
        RequestIPCBuff(caller, info);
        break;
    case SofaRequestCap_Endpoint:
        RequestCapEndoint(caller, info);
        break;
    case SofaRequestCap_NewPage:
        RequestCapNewPage(caller, info);
        break;

    default:
        assert(0);
        break;
    }
}