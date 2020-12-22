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


void Syscall_munmap(Thread* caller, seL4_MessageInfo_t info)
{
    void * addr = (void*) seL4_GetMR(1);
    size_t length = seL4_GetMR(2);//, length);

    const size_t numPages = length / 4096;
    sel4utils_unmap_pages(&caller->_base.process->native.vspace, addr, numPages, PAGE_BITS_4K, VSPACE_FREE);
    seL4_SetMR(1, 0);
    seL4_Reply(info);
}

void Syscall_mmap(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->_base.process;
    assert(process);
    
    void * addr = (void*) seL4_GetMR(1); // NULL means anywhere
    size_t length = seL4_GetMR(2);//, length);
    int prot = seL4_GetMR(3);//, prot);
    int flags = seL4_GetMR(4);//, flags);
    int fd = seL4_GetMR(5);//, fd);
    off_t offset = seL4_GetMR(6);//, offset);


    if(addr)
    {
        KLOG_INFO("mmap Only support NULL addr for now\n");
        seL4_SetMR(1, -EINVAL);
        seL4_Reply(info);
        return;

    }
    const size_t numPages = length / 4096;

    KernelTaskContext* ctx = getKernelTaskContext();
    void* p = sel4utils_new_pages(&caller->_base.process->native.vspace, seL4_AllRights, numPages, PAGE_BITS_4K);
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
