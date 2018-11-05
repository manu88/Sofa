#pragma once

#include"Bootstrap.h"


typedef struct _chardev_t 
{
    /* platsupport char device */
    ps_chardevice_t dev;
    /* IRQHandler cap (with cspace path) */
    cspacepath_t handler;
    /* endpoint cap (with cspace path) device is waiting for IRQ */
    cspacepath_t ep;

//    vka_object_t  ntfn_object;
} chardev_t;


int DriverKitInit(InitContext* context);
