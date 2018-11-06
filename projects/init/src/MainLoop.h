#pragma once

#include "Bootstrap.h"
#include "ProcessDef.h"

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER    (1 << 0)
#define IRQ_BADGE_NETWORK  (1 << 1)
#define IRQ_BADGE_KEYBOARD (1 << 2)

void handle_cdev_event( void* dev); 

int UpdateTimeout(InitContext* context,uint64_t timeNS);

// never returns
void processLoop(InitContext* context, seL4_CPtr epPtr, void* chardev );

