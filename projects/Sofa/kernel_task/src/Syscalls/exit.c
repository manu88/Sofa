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
#include <Sofa.h>
#include "SyscallTable.h"
#include "Process.h"
#include "KThread.h"


void Syscall_exit(Thread* caller, seL4_MessageInfo_t info)
{
    int retCode = seL4_GetMR(1);

    ThreadBase* base = (ThreadBase*) caller;
    if(base->kernTaskThread)
    {
        KLOG_INFO("Exit from kernel_task thread with status %i\n", retCode);
        KThreadCleanup((KThread*) caller);
    }
    else
    {
        Process* process = caller->_base.process;
        doExit(process, MAKE_EXIT_CODE(retCode, 0));
    }
}