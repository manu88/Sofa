/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Serial.h"
#include "Environ.h"
#include "Log.h"
#include <sel4platsupport/arch/io.h>
#include <vka/capops.h>
#include <utils/circular_buffer.h>


static char _circ_buffer[sizeof(circ_buf_t) + SERIAL_CIRCULAR_BUFFER_SIZE -1]; // '-1' 'cause 1 byte is already present in circ_buf_t


typedef struct 
{
    OnBytesAvailable waiter;

    size_t size;
    char until;
    void* ptr;

} SerialWaiter;

typedef struct
{
    OnControlChar waitCtl;
    void *ptr;
} SerialController;


static SerialWaiter _waiter = {0};
static SerialController _controller = {0};


static circ_buf_t* getCircularBuffer()
{
    return (circ_buf_t *) _circ_buffer;
}

int SerialInit()
{
    KernelTaskContext* env = getKernelTaskContext();

    ps_chardevice_t* r = ps_cdev_init(PC99_SERIAL_COM1 , &env->ops ,&env->comDev);
    if(r == NULL)
    {
        return -1;
    }

    int err = circ_buf_init(SERIAL_CIRCULAR_BUFFER_SIZE, getCircularBuffer());
    if(err != 0)
    {
        return err;
    }

  
    int irqNum = -1;
    for (int i=0;i<256;i++)
    {
        if(ps_cdev_produces_irq(&env->comDev, i))
        {
            irqNum = i;
            break;
        }
    }
    KLOG_DEBUG("COM DEV PRODUCES IRQ %i\n",irqNum);
    return 0;
    // FIXME: can't get the serial IRQ to work on pc99 :( :(
#if 0  
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
#endif
}

size_t SerialGetAvailableChar()
{
    return getCircularBuffer()->tail - getCircularBuffer()->head;
}


int SerialRegisterWaiter(OnBytesAvailable callback, size_t forSize, char until, void* ptr)
{
    _waiter.waiter = callback;
    _waiter.size = forSize;
    _waiter.ptr = ptr;
    _waiter.until = until;
    return 0;
}

int SerialRegisterController(OnControlChar callback, void* ptr)
{
    _controller.waitCtl = callback;
    _controller.ptr = ptr;
}

size_t SerialCopyAvailableChar(char* dest, size_t maxSize)
{
    size_t copied = 0;
    while (circ_buf_is_empty(getCircularBuffer()) == 0)
    {
        dest[copied] = circ_buf_get(getCircularBuffer());        
        copied +=1;
        if(copied > maxSize)
        {
            break;
        }
    }

    return copied;
    
}

void handleSerialInput(KernelTaskContext* env)
{
    int data = 0;
    while(data != EOF)
    {  
        data = ps_cdev_getchar(&env->comDev);
        if(data > 0)
        {
            if(data == '\03')
            {
                if(_controller.waitCtl)
                {
                    _controller.waitCtl(data, _controller.ptr);
                    continue;
                }


            }
            if(data == '\r')
                data = '\n';

            circ_buf_put(getCircularBuffer(), data);

            if(_waiter.waiter &&  _waiter.size)
            {
                // echo only if someone is waiting on us!
                putchar(data);
                fflush(stdout);
                //
                if(_waiter.until && data == _waiter.until)
                {
                    _waiter.waiter(SerialGetAvailableChar(), _waiter.until, _waiter.ptr);
                    memset(&_waiter, 0, sizeof(_waiter));
                }
                else if(SerialGetAvailableChar() >= _waiter.size)
                {
                    _waiter.waiter(SerialGetAvailableChar(), (char) 0, _waiter.ptr);
                    memset(&_waiter, 0, sizeof(_waiter));
                }
            }

        }
    }
}
