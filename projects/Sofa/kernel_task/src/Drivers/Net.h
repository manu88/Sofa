#pragma once
#include <lwip/netif.h>
#include "LList.h"
#include "KThread.h"

typedef struct
{
    LList rxList;
    KMutex rxListMutex;
} NetBuffer;

//NetBuffer *getRXBuffer(void);

typedef void (*netif_handle_irq_fn)(void *state, int irq_num);

typedef struct
{
    int irq_num;
    netif_init_fn init_fn;
    void *driver;
    netif_handle_irq_fn handle_irq_fn;

}NetworkDriver;



void NetInit(uint32_t iobase0);

//void NetSetEndpoint(ThreadBase *caller, void* buff, size_t sizeToRead);
//size_t NetSendPbuf(void* pbuf, void* cltBuf, size_t size);

