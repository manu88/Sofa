#include "Serial.h"
#include "Environ.h"
#include <vka/capops.h>

int SerialInit()
{
    KernelTaskContext* env = getKernelTaskContext();

    sel4platsupport_get_io_port_ops(&env->ops.io_port_ops, &env->simple, &env->vka);
    ps_chardevice_t* r = ps_cdev_init(PC99_SERIAL_COM1 , &env->ops ,&env->comDev);
    if(r == NULL)
    {
        return -1;
    }
    return 0;
    int irqNum = -1;
    for (int i=0;i<256;i++)
    {
        if(ps_cdev_produces_irq(&env->comDev, i))
        {
            irqNum = i;
            break;
        }
    }
    printf("COM DEV PRODUCES IRQ %i\n",irqNum);
    seL4_CPtr cap;
    int err = vka_cspace_alloc(&env->vka, &cap);
    assert(err == 0);
    vka_cspace_make_path(&env->vka, cap, &env->handler);
    err = simple_get_IRQ_handler(&env->simple, irqNum, env->handler);
    assert(err == 0);
    printf("Got IRQ handler\n");


    cspacepath_t ep;
    err = vka_cspace_alloc_path(&env->vka, &ep);
    assert(err == 0);

    cspacepath_t root_notification_path = {0};
    vka_cspace_make_path(&env->vka, env->timer_notification.cptr, &root_notification_path);

    err = vka_cnode_mint(&ep, &root_notification_path, seL4_AllRights, SERIAL_BADGE );
    assert(err == 0);

    err = seL4_IRQHandler_SetNotification(env->handler.capPtr, ep.capPtr);
    assert(err == 0);

    ps_cdev_getchar(&env->comDev);
    err = seL4_IRQHandler_Ack(env->handler.capPtr);
    assert(err == 0);
    return 0;
}

