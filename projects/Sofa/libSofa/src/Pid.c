#include "Sofa.h"
#include "proc_ctx.h"

extern ProcessContext* _ctx;

pid_t getpid()
{
    return _ctx->pid;
}

//pid_t getppid(void)