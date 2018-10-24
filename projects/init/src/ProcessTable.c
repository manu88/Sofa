#include "ProcessTable.h"
#include <utils/list.h>

static list_t _processes;


pid_t getNextPid()
{
    static pid_t accum = 2; // 1 is reserved for init
    return accum++;
}


int ProcessTableInit()
{ 
    return list_init(&_processes) == 0;
}


int ProcessTableGetCount()
{
    return list_length(&_processes);
}

int ProcessTableAppend( Process* process)
{
	return list_append(&_processes , process) == 0;
}

Process* ProcessTableGetByPID( pid_t pid)
{
    list_t *l = &_processes;

    for (struct list_node *n = l->head; n != NULL; n = n->next) 
    {
        Process* p = (Process*) n->data;
        
        if(p->_pid == pid)
        {
            return p;
        }
    }

	return NULL;
}


static int PtrComp(void*a, void*b)
{
    return a != b;
}

int ProcessTableRemove(Process* process)
{
    return list_remove(&_processes , process,PtrComp) == 0;
}



int ProcessTableSignalStop(Process* process)
{
    UNUSED int error = 0;

    

    return error;
}

