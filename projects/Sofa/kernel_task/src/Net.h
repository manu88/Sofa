#pragma once
#include <lwip/netif.h>


typedef void (*netif_handle_irq_fn)(void *state, int irq_num);

typedef struct
{
    int irq_num;
    netif_init_fn init_fn;
    void *driver;
    netif_handle_irq_fn handle_irq_fn;

}NetworkDriver;



void NetInit(uint32_t iobase0);