#pragma once



#include "ProcessDef.h"



int ProcessTableInit(void);

int ProcessTableAppend( Process* process);

Process* ProcessTableGetByPID( pid_t pid);

int ProcessTableRemove(Process* process);

int ProcessTableGetCount(void);



pid_t getNextPid(void);
