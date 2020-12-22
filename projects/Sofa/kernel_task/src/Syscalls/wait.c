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
#include "SyscallTable.h"
#include "Process.h"
#include "utils.h"



void Syscall_wait(Thread* caller, seL4_MessageInfo_t info)
{
    Process* process = caller->_base.process;
    KernelTaskContext* env = getKernelTaskContext();

    pid_t pidToWait = seL4_GetMR(1);
    int options = seL4_GetMR(2);

    // does the process have any children?
    if(ProcessCoundChildren(process) == 0)
    {
        seL4_SetMR(1, -1);
        seL4_SetMR(2, 0); // wstatus
        seL4_Reply(info);
        return;
    }

    // Do we have zombie children?

    Process* c = NULL;
    PROCESS_FOR_EACH_CHILD(process, c)
    {
        if(c->state == ProcessState_Zombie)
        {
            break;
        }
    }
    if(c)
    {
        seL4_SetMR(1, ProcessGetPID(c));
        seL4_SetMR(2, c->retCode); // wstatus

        ProcessRemoveChild(process, c);
        ProcessListRemove(c);
        kfree(c);        

        seL4_Reply(info);
        return;
    }

    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        KLOG_TRACE("[Syscall_wait] Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }
    caller->_base.replyCap = slot;
    caller->state = ThreadState_Waiting;

}