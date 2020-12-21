#include <Sofa.h>
#include "SyscallTable.h"
#include "Reboot.h"


void SysCall_Reboot(Thread* caller, seL4_MessageInfo_t info)
{
    RebootMode mode = seL4_GetMR(1);
    KLOG_INFO("Syscall reboot, code %i\n", mode);

    int ret = -1;
    if(mode == RebootMode_Shutdown)
    {
        ret = 0;
    }

    seL4_SetMR(1, ret);
    seL4_Reply(info);

    if(mode == RebootMode_Shutdown)
    {
        doShutdown();
    }
} 