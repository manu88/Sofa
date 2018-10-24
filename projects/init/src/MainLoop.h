#pragma once

#include "Bootstrap.h"
#include "ProcessDef.h"



#define APP_PRIORITY seL4_MaxPrio

void processSyscall(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message, seL4_Word badge);

int UpdateTimeout(InitContext* context,uint64_t timeNS);
