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
#include "utils.h"
#include "Panic.h"
#include "NameServer.h"
#include "DeviceTree.h"


int _do_traces = 0;

static char getProcessStateStr(ProcessState s)
{
    switch (s)
    {
    case ProcessState_Running:
        return 'R';
    case ProcessState_Zombie:
        return 'Z';
    
    default:
        assert(0);
    }
}

void Syscall_Debug(Thread* caller, seL4_MessageInfo_t info)
{
    Process* proc = caller->_base.process;

    SofaDebugCode op = seL4_GetMR(1);
    switch (op)
    {
    case SofaDebugCode_EnableSyscallTraces:
        if(_do_traces == 0)
        {
            _do_traces = 1;
        }
        else
        {
            _do_traces = 0;
        }
        KLOG_INFO("Set sycall traces to %i\n", _do_traces);
        break;
    case SofaDebugCode_DumpSched:
        seL4_DebugDumpScheduler();
        break;
    case SofaDebugCode_ListServices:
    {
        Service* s = NULL;
        Service* tmp = NULL;
        FOR_EACH_SERVICE(s, tmp)
        {
            printf("'%s' owner %i %s\n", s->name, ProcessGetPID(s->owner), ProcessGetName(s->owner));
        }
    }
        break;    
    default:
        break;
    }


}