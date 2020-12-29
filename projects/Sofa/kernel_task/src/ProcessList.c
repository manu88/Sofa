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
#include "ProcessList.h"
#include "Log.h"

static Process* _processes = NULL;

Process* getProcessList()
{
    return _processes;
}

void ProcessListAdd(Process* p)
{
    LL_APPEND(_processes, p);
}

size_t ProcessListCount()
{
    Process* el = NULL;
    size_t c = 0;
    LL_COUNT(_processes, el, c);
    return c;
}

void ProcessListRemove(Process* p)
{
    LL_DELETE(_processes, p);
}


Process* ProcessListGetByPid(pid_t pid)
{
    Process* p= NULL;
    FOR_EACH_PROCESS(p)
    {
        if(ProcessGetPID(p) == pid)
        {
            return p;
        }
    }
    return NULL;
}



int ProcessCountExtraThreads(const Process* p)
{
    int count = 0;
    Thread* el = NULL;
    LL_COUNT(p->threads, el, count);
    return count;
}


Thread* ProcessGetWaitingThread(Process*p)
{
    if(p->main._base.state == ThreadState_Waiting)
    {
        return &(p->main);
    }
    Thread* t = NULL;
    PROCESS_FOR_EACH_EXTRA_THREAD(p, t)
    {
        if(t->_base.state == ThreadState_Waiting)
        {
            return t;
        }
    }
    return NULL;
}


void ProcessAddChild(Process* parent, Process* child)
{
    assert(parent != child);
    child->parent = parent;
    LL_APPEND2(parent->children, child, nextChild);
}

void ProcessRemoveChild(Process* parent, Process* child)
{
    LL_DELETE2(parent->children, child, nextChild);
}

int ProcessCoundChildren(const Process* p)
{
    int counter = 0;
    Process* tmp = NULL;
    LL_COUNT2(p->children, tmp, counter, nextChild);
    return counter;
}