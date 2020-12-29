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
#include "Process.h"
#include "utils.h"

long doWait(Process* process, pid_t pidToWait, int options, int *retCode)
{
// does the process have any children?
    if(ProcessCoundChildren(process) == 0)
    {
        return -ECHILD;
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
        if(retCode)
        {
            *retCode = c->retCode;
            pid_t pid = ProcessGetPID(c);
            ProcessRemoveChild(process, c);
            ProcessListRemove(c);
            kfree(c);
            return pid;
        }
        return;
    }
// Process has children, but no-one exited yet.
    return -EWOULDBLOCK;
}
