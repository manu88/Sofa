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