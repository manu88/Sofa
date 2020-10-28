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

void Process_AddChild(Process* parent, Process* child)
{
    HASH_ADD(hchld, parent->children, pid, sizeof(int), child);
    child->parent = parent;
}

void Process_RemoveChild(Process* parent, Process* child)
{
    HASH_DELETE(hchld, parent->children, child);
    child->parent = NULL;
}

size_t Process_CountChildren( const Process* p)
{
    return HASH_CNT(hchld, p->children);
    //return HASH_COUNT(p->children);
}

int Process_IsWaiting(const Process* p)
{
    return p->_isWaiting;
}