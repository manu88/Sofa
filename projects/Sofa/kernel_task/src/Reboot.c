#include "Reboot.h"
#include "Environ.h"
#include "Log.h"
#include "testtypes.h"
#include "Process.h"
#include "NameServer.h"


extern Process initProcess;

void doShutdown(void)
{
    KernelTaskContext* ctx = getKernelTaskContext();

    KLOG_INFO("System will shutdown\n");
    ctx->_sysState = SystemState_Halting;

// Suspend the init process so that it does not spawn any new console
    process_suspend(&initProcess); 

// Kill all processes. Hardcore
    Process* p = NULL;
    FOR_EACH_PROCESS(p)
    {
        if(p != &initProcess)
        {
            KLOG_DEBUG("Stopping process %i %s\n", ProcessGetPID(p), ProcessGetName(p));
            doExit(p, -1);
        }
    }

    seL4_DebugDumpScheduler();


// Stop all NameServices
    Service* serv = NULL;
    Service* tmp = NULL;

    FOR_EACH_SERVICE(serv, tmp)
    {
        KLOG_INFO("sending service %s a will stop\n", serv->name);
        
        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
        seL4_SetMR(0, ServiceNotification_WillStop);
        seL4_Send(serv->kernTaskEp, info);

    }


    KLOG_INFO("Did Send will stop msg to services\n");
//    ps_io_port_out(&ctx->ops.io_port_ops, 0x604, 2, 0x2000);

    
}



void doReboot()
{
}