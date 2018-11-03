#pragma once


#include "Sofa.h"
#include "ProcessDef.h"



int ProcessTableInit(void) SOFA_UNIT_TESTABLE;

int ProcessTableAppend( Process* process) SOFA_UNIT_TESTABLE;

Process* ProcessTableGetByPID( pid_t pid) SOFA_UNIT_TESTABLE;

int ProcessTableRemove(Process* process) SOFA_UNIT_TESTABLE;

int ProcessTableGetCount(void) SOFA_UNIT_TESTABLE;

pid_t ProcessTableGetNextPid(void);
