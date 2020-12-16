#include "SyscallTable.h"
#include <sel4utils/vspace_internal.h>


void Syscall_munmap(Thread* caller, seL4_MessageInfo_t info)
{
    void * addr = seL4_GetMR(1);
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
    
    void * addr = seL4_GetMR(1); // NULL means anywhere
    size_t length = seL4_GetMR(2);//, length);
    int prot = seL4_GetMR(3);//, prot);
    int flags = seL4_GetMR(4);//, flags);
    int fd = seL4_GetMR(5);//, fd);
    off_t offset = seL4_GetMR(6);//, offset);


    KLOG_DEBUG("mmap call with addr=%p length=%zi prot=%i flags=%X fd=%i offset=%i\n",
            addr, length, prot, flags, fd, offset);

    if(addr)
    {
        KLOG_INFO("mmap Only support NULL addr for now\n");
        seL4_SetMR(1, -EINVAL);
        seL4_Reply(info);
        return;

    }
    const size_t numPages = length / 4096;
    KLOG_DEBUG("Alloc %zi page(s)\n", numPages);
    KernelTaskContext* ctx = getKernelTaskContext();
    void* p = sel4utils_new_pages(&caller->_base.process->native.vspace, seL4_AllRights, numPages, PAGE_BITS_4K);
    //void* p = vspace_new_pages(&caller->_base.process->native.vspace, seL4_AllRights, numPages, PAGE_BITS_4K);
/*    
    void* pp = (uint8_t*) vspace_new_pages(&ctx->vspace, seL4_AllRights, numPages, PAGE_BITS_4K);
    assert(pp);
    void*p  = vspace_share_mem(&ctx->vspace, &process->native.vspace, pp, numPages, PAGE_BITS_4K, seL4_AllRights, 1);
    assert(p);
*/
    if(!p)
    {
        KLOG_DEBUG("unable to create a new page\n");
        seL4_SetMR(1, -ENOMEM);
        seL4_Reply(info);
        return;
    }
    //memset(p, 0, numPages * 4096);
    KLOG_DEBUG("new page ok %p\n", p);

    seL4_SetMR(1,(seL4_Word) p);
    seL4_Reply(info);   
}
