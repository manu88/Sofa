#include "SyscallTable.h"
#include "../utils.h"
#include "Log.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>

static int sleep_callback(uintptr_t token)
{
    Thread* caller = (Thread*) token;
    assert(caller);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Sleep);
    seL4_SetMR(1, 0); // sucess

    seL4_Send(caller->_base.replyCap, tag);

    ThreadCleanupTimer(caller);
    return 0;
}


void Syscall_sleep(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    assert(caller->_base.replyCap == 0);

    unsigned int timerID = 0;
    int error = tm_alloc_id(&env->tm, &timerID);
    if (error)
    {
        KLOG_TRACE("Unable to alloc timer id err=%i\n", error);

        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }


    seL4_Word slot = get_free_slot(&env->vka);
    error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        KLOG_TRACE("Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        tm_free_id(&env->tm, timerID);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    error = tm_register_cb(&env->tm, TIMEOUT_RELATIVE, seL4_GetMR(1) * NS_IN_MS, 0, timerID , sleep_callback, (uintptr_t) caller);
    if(error)
    {
        KLOG_TRACE("tm_register_failed, err=%i\n", error);
        tm_free_id(&env->tm, timerID);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    caller->_base.currentSyscallID = SyscallID_Sleep;
    caller->_base.timerID = timerID;
    caller->_base.replyCap = slot;
}