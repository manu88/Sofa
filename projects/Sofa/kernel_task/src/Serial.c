#include "Serial.h"
#include "Environ.h"


int SerialInit()
{
    KernelTaskContext* env = getKernelTaskContext();
    sel4platsupport_get_io_port_ops(&env->ops.io_port_ops, &env->simple, &env->vka);
    ps_chardevice_t* r = ps_cdev_init(PC99_SERIAL_COM1 , &env->ops ,&env->comDev);
    if(r == NULL)
    {
        return -1;
    }
    for (int i=0;i<256;i++)
    {
        if(ps_cdev_produces_irq(&env->comDev, i))
        {
            printf("COM DEV PRODUCES IRQ %i\n",i);
        }

    }


    return 0;
}

