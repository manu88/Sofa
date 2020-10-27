#include "Process.h"

static size_t _pid = 1;


static Process *_processes = NULL;

void ProcessInit(Process*p)
{
    memset(p, 0, sizeof(Process));
}

size_t Process_GetNextPID()
{
    return _pid++;
}

Process* Process_GetByPID(int pid)
{
    Process* p = NULL;
    HASH_FIND_INT(_processes, &pid, p);
    return p;
}

void Process_Add(Process* p)
{
    HASH_ADD_INT(_processes, pid, p);
}

void Process_Remove(Process* p)
{
    HASH_DEL(_processes, p);
}