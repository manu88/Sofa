/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "SyscallTable.h"
#include "Process.h"
#include <Sofa.h>
#include <vka/capops.h>


static seL4_CPtr CreateCapEndoint(Thread* caller)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    cspacepath_t res;
    vka_cspace_make_path(&getKernelTaskContext()->vka, ctx->root_task_endpoint.cptr, &res);
    seL4_CPtr ret = sel4utils_mint_cap_to_process(&caller->_base.process->native, res, seL4_AllRights, (seL4_Word) caller); 
    //seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &getKernelTaskContext()->vka);
    return ret;
}

static seL4_CPtr CreateCapTCB(Thread* caller, vka_object_t* tcb)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    int err = vka_alloc_tcb(&getKernelTaskContext()->vka, tcb);
    if(err != 0)
    {
        return 0;
    }
    assert(tcb->cptr);
    //cspacepath_t res;
    //vka_cspace_make_path(&ctx->vka, tcb->cptr, &res);
    seL4_CPtr ret = sel4utils_copy_cap_to_process(&caller->_base.process->native, &ctx->vka, tcb->cptr);
    //seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &ctx->vka);
    return ret;
}

static void* CreateNewStack(Thread* caller)
{
    KernelTaskContext* env = getKernelTaskContext();
    void* p = vspace_new_sized_stack(&caller->_base.process->native.vspace, 1);
    
    return p;
}

static void* CreateIPCBuff(Thread* caller, seL4_CPtr* cap)
{
    Process* process = caller->_base.process;
    assert(process);
    KernelTaskContext* env = getKernelTaskContext();
    seL4_CPtr buf;
    void* bufAddr = vspace_new_ipc_buffer(&caller->_base.process->native.vspace, &buf);

    cspacepath_t res;
    vka_cspace_make_path(&getKernelTaskContext()->vka, buf, &res);
    seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &getKernelTaskContext()->vka);

    *cap = ret;
    return bufAddr;
}

void Syscall_ThreadNew(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    Process* process = caller->_base.process;
    KLOG_DEBUG("RequestNewThread2 from %i\n", ProcessGetPID(process));

    /*
    MR 1 status : 0 ok
       2 tcb
       3 ep
       4 ipcbufcap
       5 ipcbuf
       6 stacktop
       7 sofa IPC buffer
    */

    Thread* newThread = kmalloc(sizeof(Thread));
    if(newThread == NULL)
    {
        seL4_SetMR(1, -ENOMEM);
        seL4_Reply(info);
        return;
    }

    memset(newThread, 0, sizeof(Thread));
    newThread->_base.process = process;

    seL4_CPtr tcb = CreateCapTCB(caller, &newThread->tcb);
    assert(tcb);
    seL4_CPtr ep = CreateCapEndoint(newThread);
    assert(ep);
    seL4_CPtr ipcBuf = 0;
    void* ipcBufAddr = CreateIPCBuff(caller, &ipcBuf);
    KLOG_DEBUG("ipcBufAddr %p\n", ipcBufAddr);
    KLOG_DEBUG("ipcBuf %lu\n", ipcBuf);
    assert(ipcBufAddr);
    assert(ipcBuf);
    void* stacktop = CreateNewStack(caller);
    KLOG_DEBUG("stacktop %p\n", stacktop);
    assert(stacktop);

    void* addr = NULL;
    void* procAddr = NULL;
    ProcessCreateSofaIPCBuffer(process, &addr, &procAddr);
    KLOG_DEBUG("sofa ipc addr %p\n", addr);
    KLOG_DEBUG("sofa ipc proc addr %p\n", procAddr);
    assert(addr);
    assert(procAddr);

    newThread->stack = stacktop;
    newThread->stackSize = 1;
    newThread->ipcBuffer = addr;
    newThread->ipcBuffer_vaddr = procAddr;
    newThread->ipcBuf2 = ipcBufAddr;
    LL_APPEND(process->threads, newThread);

    seL4_SetMR(1, (seL4_Word) 0);
    seL4_SetMR(2, (seL4_Word) tcb);
    seL4_SetMR(3, (seL4_Word) ep);
    seL4_SetMR(4, (seL4_Word) ipcBuf);
    seL4_SetMR(5, (seL4_Word) ipcBufAddr);
    seL4_SetMR(6, (seL4_Word) stacktop);
    seL4_SetMR(7, (seL4_Word) procAddr);
    seL4_Reply(info);
}

void Syscall_ThreadExit(Thread* caller, seL4_MessageInfo_t info)
{
    int ret = (int) seL4_GetMR(1);
    KLOG_DEBUG("Thread exit request code %i\n", ret);
    Process* process = caller->_base.process;
    assert(process);

    KLOG_DEBUG("start vspace_unmap_pages\n");
    vspace_unmap_pages(&process->native.vspace, caller->ipcBuffer_vaddr, 1, PAGE_BITS_4K, VSPACE_FREE);
    KLOG_DEBUG("end vspace_unmap_pages\n");

    seL4_TCB_Suspend(caller->tcb.cptr);
    KLOG_DEBUG("start vspace_free_ipc_buffer\n");
    vspace_free_ipc_buffer(&getKernelTaskContext()->vspace, (seL4_Word *) caller->ipcBuf2);
    KLOG_DEBUG("end vspace_free_ipc_buffer\n");
    LL_DELETE(process->threads, caller);
    kfree(caller);
}