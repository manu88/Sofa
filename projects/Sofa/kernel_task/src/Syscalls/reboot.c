#include <Sofa.h>
#include "SyscallTable.h"

void SysCall_Reboot(Thread* caller, seL4_MessageInfo_t info)
{
    RebootMode mode = seL4_GetMR(1);
    KLOG_INFO("Syscall reboot, code %i\n", mode);

    int ret = 0;

    seL4_SetMR(1, ret);
    seL4_Reply(info);

    KernelTaskContext* ctx = getKernelTaskContext();
    ps_io_port_out(&ctx->ops.io_port_ops, 0x604, 2, 0x2000);

}