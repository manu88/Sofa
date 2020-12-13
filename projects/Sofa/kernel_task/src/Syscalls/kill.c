#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"

void Syscall_Kill(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* process = caller->_base.process;

    pid_t pidToKill = seL4_GetMR(1);

    int signal = seL4_GetMR(2);

    int ret = 0;

    Process* processToKill = ProcessListGetByPid(pidToKill);
    if(processToKill == NULL)
    {
        ret = -ESRCH;
    }
    else if(processToKill->state == ProcessState_Zombie)
    {
        ret = 0;
    }
    else
    {
        pid_t callerPid = ProcessGetPID(process);
        doExit(processToKill, signal);        
        ret = 0;

        // did we just killed yourselves?
        if(pidToKill == callerPid)
        {
            // dont bother send a reply, we're dead!
            return;
        }
    }

    info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Kill);

    seL4_SetMR(1, ret);
    seL4_Reply(info);
}
