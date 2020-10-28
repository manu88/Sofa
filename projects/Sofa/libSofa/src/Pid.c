#include "Sofa.h"
#include "proc_ctx.h"
#include "sys_calls.h"

extern ProcessContext* _ctx;

pid_t getpid()
{
    return _ctx->pid;
}

pid_t getppid(void)
{
    TLSContext* ctx = (TLSContext*)seL4_GetUserData();
    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
    seL4_SetMR(0, SofaSysCall_PPID);

    seL4_Call(ctx->endpoint, msg);

    return seL4_GetMR(1);
}

pid_t wait(int *wstatus)
{
    return waitpid(-1, wstatus, 0);
}

pid_t waitpid(pid_t pid, int *wstatus, int options)
{
    TLSContext* ctx = (TLSContext*)seL4_GetUserData();
    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,3);
    seL4_SetMR(0, SofaSysCall_Wait);
    seL4_SetMR(1, pid);
    seL4_SetMR(2, options);  
    seL4_Call(ctx->endpoint, msg);

    int retPid = seL4_GetMR(1);
    int status = seL4_GetMR(2);

    *wstatus = status;
    return retPid;
}