#include "ProcessDef.h"

Process* ProcessAlloc()
{
    Process* p = malloc(sizeof(Process));

    ProcessInit(p);
    return p;
}

int ProcessInit(Process* process)
{
    memset(process , 0 , sizeof(Process) );
    return 1;
}


int ProcessRelease(Process* process)
{
    free(process);
    return 1;
}
