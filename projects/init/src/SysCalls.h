#pragma once


#include "Bootstrap.h"
#include "ProcessDef.h"

typedef int (*SysCallHandler)(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);



int handle_nanosleep(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_getpid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_getppid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_exit(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_kill(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_execve(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_wait4(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
