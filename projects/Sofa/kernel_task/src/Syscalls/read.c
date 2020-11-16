#include "SyscallTable.h"
#include "../utils.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>




void Syscall_read(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    int c = ps_cdev_getchar(&env->comDev);

    seL4_SetMR(1, (seL4_Word) c);
    seL4_Reply(info);
}