/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Reboot.h"
#include "Environ.h"
#include "Log.h"
#include "Process.h"
#include "ProcessList.h"
#include "NameServer.h"
#include "DeviceTree.h"

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
    ProcessListLock();
    FOR_EACH_PROCESS(p)
    {
        if(p != &initProcess)
        {
            KLOG_DEBUG("Stopping process %i %s\n", ProcessGetPID(p), ProcessGetName(p));
            doExit(p, -1);
        }
    }
    ProcessListUnlock();

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
    KLOG_INFO("Send hardware shutdown\n");

    DeviceTreeHardwarePoweroff();
}



void doReboot()
{

}
