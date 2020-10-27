#include <errno.h>
#include "Spawn.h"
#include "proc_ctx.h"
#include "sys_calls.h"

extern ProcessContext* _ctx;

int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[])
{   
    TLSContext* ctx = (TLSContext*)seL4_GetUserData();

    const size_t filePathLen = strlen(file);
    if(filePathLen > IPC_BUF_LEN)
    {
        return -ENAMETOOLONG;
    }
    strncpy(_ctx->ipcBuffer, file, filePathLen);
    _ctx->ipcBuffer[filePathLen] = 0;
    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,3);
    seL4_SetMR(0, SofaSysCall_Spawn);
    seL4_SetMR(1, filePathLen);
    seL4_Call(ctx->endpoint, msg);

    seL4_Word ret = seL4_GetMR(1);
    if(ret > 0)
    {
        *pid = (pid_t) ret;
        return 0;
    }

    return (int) ret;
}