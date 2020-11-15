#include "Process.h"


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



int ProcessCountExtraThreads(const Process* p)
{
    int count = 0;
    Thread* el = NULL;
    LL_COUNT(p->threads, el, count);
    return count;
}


Thread* ProcessGetWaitingThread( const Process*p)
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