#include "Reboot.h"
#include "Environ.h"
#include "Log.h"


void doShutdown(void)
{
    KLOG_INFO("System will shutdown\n");
    KernelTaskContext* ctx = getKernelTaskContext();
    ps_io_port_out(&ctx->ops.io_port_ops, 0x604, 2, 0x2000);

}

void doReboot()
{

}