#pragma once

#include "Bootstrap.h"
#include "ProcessDef.h"



#define APP_PRIORITY seL4_MaxPrio

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER   (1 << 1)
#define IRQ_BADGE_NETWORK (1 << 0)


void processSyscall(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message, seL4_Word badge);

int UpdateTimeout(InitContext* context,uint64_t timeNS);


void processLoop(InitContext* context, seL4_CPtr epPtr );

// temp
void processTimer(InitContext* context,seL4_Word sender_badge);
