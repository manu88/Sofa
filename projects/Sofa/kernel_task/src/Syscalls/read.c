#include "SyscallTable.h"
#include "../utils.h"
#include "Serial.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>


static void onControlChar(char ctl, void* ptr)
{
    Thread* caller = (Thread*) ptr;
    assert(caller);
    if (ctl == '\03')
    {
        printf("CTL-C\n");
        if(ThreadIsWaiting(caller))
        {
            seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
            seL4_SetMR(0, caller->_base.currentSyscallID);
            seL4_SetMR(1, -EINTR);
    
            seL4_Send(caller->_base.replyCap, tag);

            cnode_delete(&getKernelTaskContext()->vka, caller->_base.replyCap);
            caller->_base.replyCap = 0;
            caller->state = ThreadState_Running;
        }
    }
    else
    {
        printf("Got Control char %i\n", ctl);
    }
}

static void onBytesAvailable(size_t size, char until, void* ptr)
{
    Thread* caller = (Thread*) ptr;
    assert(caller);

    size_t bytes = SerialCopyAvailableChar(caller->ipcBuffer, size);
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
    seL4_SetMR(0, SyscallID_Read);
    seL4_SetMR(1, until?bytes: -EAGAIN);
    

    seL4_Send(caller->_base.replyCap, tag);
    cnode_delete(&getKernelTaskContext()->vka, caller->_base.replyCap);
    caller->_base.replyCap = 0;
    caller->state = ThreadState_Running;

}

void Syscall_Read(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();

    size_t sizeToRead = seL4_GetMR(1);
    char readUntil = (char)seL4_GetMR(2);

    assert(caller->_base.replyCap == 0);

    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        KLOG_TRACE("Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    caller->_base.replyCap = slot;
    caller->_base.currentSyscallID = SyscallID_Read;
    SerialRegisterWaiter(onBytesAvailable, sizeToRead, readUntil, caller);
    SerialRegisterController(onControlChar, caller);

}

void Syscall_Write(Thread* caller, seL4_MessageInfo_t info)
{
    size_t sizeToWrite = seL4_GetMR(1);
    printf("%s", caller->ipcBuffer);
    fflush(stdout);
}