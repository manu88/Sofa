#include "SyscallTable.h"
#include "../utils.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>



void on_read_complete(ps_chardevice_t* device, enum chardev_status stat, size_t bytes_transfered, void* token)
{
    Thread* caller = (Thread*) token;
    printf("on_read_complete got %zi bytes\n", bytes_transfered);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Read);
    seL4_SetMR(1, 0);

    seL4_Send(caller->replyCap, tag);

    cnode_delete(&getKernelTaskContext()->vka, caller->replyCap);
    caller->replyCap = 0;
}


void Syscall_read(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();

    printf("[Syscall_read] request\n");

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

    ssize_t r =  ps_cdev_read(&env->comDev, caller->ipcBuffer, 12, on_read_complete, caller);
    printf("AFTER ps_cdev_read %li\n", r);
/*    
    int c = ps_cdev_getchar(&env->comDev);

    seL4_SetMR(1, (seL4_Word) c);
    seL4_Reply(info);
*/
}