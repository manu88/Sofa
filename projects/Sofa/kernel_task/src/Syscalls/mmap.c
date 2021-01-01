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
#include <sel4utils/vspace_internal.h>
#include "Process.h"


void Syscall_munmap(Thread* caller, seL4_MessageInfo_t info)
{
    void * addr = (void*) seL4_GetMR(1);
    size_t length = seL4_GetMR(2);//, length);

    const size_t numPages = length / 4096;
    process_unmap_pages(caller->_base.process, addr, numPages);
    seL4_SetMR(1, 0);
    seL4_Reply(info);
}

void Syscall_mmap(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->_base.process;
    assert(process);
    
    void * addr = (void*) seL4_GetMR(1); // NULL means anywhere
    size_t length = seL4_GetMR(2);
    int prot = seL4_GetMR(3);
    int flags = seL4_GetMR(4);
    int fd = seL4_GetMR(5);
    off_t offset = seL4_GetMR(6);

    if(addr)
    {
        KLOG_INFO("mmap Only support NULL addr for now\n");
        seL4_SetMR(1, -EINVAL);
        seL4_Reply(info);
        return;

    }
    const size_t numPages = length / 4096;

    KernelTaskContext* ctx = getKernelTaskContext();
    void* p = process_new_pages(caller->_base.process, seL4_AllRights, numPages);
    if(!p)
    {
        KLOG_DEBUG("unable to create a new page\n");
        seL4_SetMR(1, -ENOMEM);
        seL4_Reply(info);
        return;
    }

    seL4_SetMR(1,(seL4_Word) p);
    seL4_Reply(info);   
}


void Syscall_shareMem(Thread* caller, seL4_MessageInfo_t info)
{
    Process* proc = caller->_base.process;

    void* addrToShare = (void* ) seL4_GetMR(1);
    Thread* procToShareWith =(Thread*) seL4_GetMR(2);
    
    KLOG_DEBUG("Syscall_shareMem %p from %i with  %i\n", addrToShare, ProcessGetPID(caller->_base.process), ProcessGetPID(procToShareWith->_base.process));

    void* sharedp = vspace_share_mem(&proc->native.vspace, &procToShareWith->_base.process->native.vspace, addrToShare, 1, PAGE_BITS_4K, seL4_AllRights, 1);
    assert(sharedp);

    KLOG_DEBUG("Shared mem at %p\n", sharedp);

    seL4_SetMR(1, sharedp);
    seL4_Reply(info);

}