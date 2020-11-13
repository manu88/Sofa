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