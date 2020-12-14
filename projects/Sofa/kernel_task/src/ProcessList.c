#include "ProcessList.h"


static Process* _processes = NULL;

Process* getProcessList()
{
    return _processes;
}

void ProcessListAdd(Process* p)
{
    LL_APPEND(_processes, p);
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
    if(p->main.state == ThreadState_Waiting)
    {
        return &(p->main);
    }
    Thread* t = NULL;
    PROCESS_FOR_EACH_EXTRA_THREAD(p, t)
    {
        if(t->state == ThreadState_Waiting)
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