#include "KThread.h"
#include "Environ.h"
#include "utils.h"
#include <string.h>
#include <vka/capops.h>
#include <Sofa.h>

/* Stub KThread instance for the main kernel_task thread, that CANNOT sleep.
Calls to KSleep will ensure that they are never called from main*/
KThread _mainThread;

void KThreadInit(KThread* t)
{
    memset(t, 0, sizeof(KThread));
    t->_base.kernTaskThread = 1;
}


static void __entry_fn(void *arg0, void *arg1, void *ipc_buf)
{
    KThread* t = (KThread*) arg0;
    assert(t);
    seL4_SetUserData(t);
    int ret = t->mainFunction(t, arg1);
    seL4_SetUserData(0);
    KThreadExit(t, ret);
    assert(0);
}

int KThreadRun(KThread* t, int prio, void* arg)
{
    int error;
    KernelTaskContext* env = getKernelTaskContext();
    sel4utils_thread_config_t thConf = thread_config_new(&env->simple);
    thConf = thread_config_cspace(thConf, simple_get_cnode(&env->simple), 0);

    // create a minted enpoint for the thread
    cspacepath_t srcPath;
    vka_cspace_make_path(&env->vka, env->root_task_endpoint.cptr, &srcPath);

    t->ep = get_free_slot(&env->vka);
    cspacepath_t dstPath;
    vka_cspace_make_path(&env->vka, t->ep, &dstPath);

    vka_cnode_mint(&dstPath, &srcPath, seL4_AllRights, (seL4_Word) t);
    printf("Set the fault ep\n");
    thConf = thread_config_fault_endpoint(thConf, t->ep);

    error = sel4utils_configure_thread_config(&env->vka,
                                              &env->vspace,
                                              &env->vspace,
                                              thConf ,
                                              &t->native);
    if(error != 0)
    {
        return error;
    }

    error = seL4_TCB_SetPriority(t->native.tcb.cptr, seL4_CapInitThreadTCB, prio);
    if(error != 0)
    {
        return error;
    }

#ifdef CONFIG_DEBUG_BUILD
    if(t->name)
    {
        seL4_DebugNameThread(t->native.tcb.cptr, t->name);
    }
#endif

    error = sel4utils_start_thread(&t->native, __entry_fn ,t , arg, 1);
    if(error != 0)
    {
        return error;
    }

    return 0;
}

void KThreadCleanup(KThread* t)
{
    KernelTaskContext* env = getKernelTaskContext();

    sel4utils_clean_up_thread(&env->vka, &env->vspace, &t->native);
    
}

int KThreadSleep(KThread* thread, int ms)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_NoFault, 0,0,2);
    seL4_SetMR(0, SyscallID_Sleep);
    seL4_SetMR(1, ms);
    seL4_Call(thread->ep, info);
    return seL4_GetMR(1);
}

int KSleep(int ms)
{
    assert(seL4_GetUserData());
    KThread* t = (KThread*) seL4_GetUserData();
    if(t == &_mainThread)
    {
        printf("KSleep called from the main kernel_task thread, abord\n");
        assert(0);
    }
    return KThreadSleep(seL4_GetUserData(), ms);
}

void KThreadExit(KThread* thread, int code)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_NoFault, 0,0,2);
    seL4_SetMR(0, SyscallID_Exit);
    seL4_SetMR(1, code);
    seL4_Call(thread->ep, info);
    assert(0);
    while(1);
}