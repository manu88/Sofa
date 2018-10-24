#pragma once

#include "Bootstrap.h"
#include "ProcessDef.h"



#define APP_PRIORITY seL4_MaxPrio

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER   (1 << 1)
#define IRQ_BADGE_NETWORK (1 << 0)



int UpdateTimeout(InitContext* context,uint64_t timeNS);

// never returns
void processLoop(InitContext* context, seL4_CPtr epPtr );

