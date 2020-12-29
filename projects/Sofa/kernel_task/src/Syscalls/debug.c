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
    KernelTaskContext* env = getKernelTaskContext();
    Process* proc = caller->_base.process;

    SofaDebugCode op = seL4_GetMR(1);
    switch (op)
    {
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