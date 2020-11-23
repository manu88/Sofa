#include "SyscallTable.h"
#include "../utils.h"
#include "../Serial.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>


/*
void on_read_complete(ps_chardevice_t* device, enum chardev_status stat, size_t bytes_transfered, void* token)
{
    Thread* caller = (Thread*) token;
    printf("on_read_complete got %zi bytes %zi are available\n", bytes_transfered, SerialGetAvailableChar());

    size_t bytes = SerialCopyAvailableChar(caller->ipcBuffer, bytes_transfered);
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Read);
    seL4_SetMR(1, bytes);

    seL4_Send(caller->replyCap, tag);

    cnode_delete(&getKernelTaskContext()->vka, caller->replyCap);
    caller->replyCap = 0;
}
*/


static void onBytesAvailable(size_t size, char until, void* ptr)
{
    Thread* caller = (Thread*) ptr;
    assert(caller);

    size_t bytes = SerialCopyAvailableChar(caller->ipcBuffer, size);
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
    seL4_SetMR(0, SyscallID_Read);
    seL4_SetMR(1, until?bytes: -EAGAIN);
    

    seL4_Send(caller->replyCap, tag);

    cnode_delete(&getKernelTaskContext()->vka, caller->replyCap);
    caller->replyCap = 0;

}

void Syscall_Read(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();

    size_t sizeToRead = seL4_GetMR(1);
    char readUntil = (char)seL4_GetMR(2);

    assert(caller->replyCap == 0);

    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        printf("Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    caller->replyCap = slot;
    SerialRegisterWaiter(onBytesAvailable, sizeToRead, readUntil, caller);

}


void Syscall_Write(Thread* caller, seL4_MessageInfo_t info)
{
    size_t sizeToWrite = seL4_GetMR(1);
    printf("%s", caller->ipcBuffer);
    fflush(stdout);
}