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
#include "IODevice.h"
#include "Log.h"
#include "KThread.h"
#include "Environ.h"

ssize_t IODeviceRead(IODevice* dev, size_t sector, char* buf, size_t bufSize)
{
    return dev->ops->read(dev, sector, buf, bufSize);
}

ssize_t IODeviceWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize)
{
    return dev->ops->write(dev, sector, buf, bufSize);
}

static const char tName[] = "IRQServer";
static KThread _irqThread;
static int once = 1;

static seL4_CPtr _irqaep = 0;
static int _irq = -1;
static IODevice *_dev = NULL;


static int _irqMain(KThread* th, void* arg)
{
    KLOG_INFO("Starting IRQ Server\n");
    assert(_irqaep);
    assert(_irq);
    assert(_dev);

    while (1)
    {
        seL4_Wait(_irqaep,NULL);
        seL4_IRQHandler_Ack(_irq);
        _dev->ops->handleIRQ(_dev, _irq);
    }
    
}

int IODeviceRegisterIRQ(IODevice* dev, int irqN)
{
    if(once != 1)
    {
        return -1;
    }
    once = 0;
    KLOG_INFO("IODeviceRegisterIRQ %i for '%s'\n", irqN, dev->name);

    KernelTaskContext* context = getKernelTaskContext();

    cspacepath_t irq_path = { 0 };
    seL4_CPtr irq;

    int error = vka_cspace_alloc(&context->vka, &irq);
    assert(error == 0);
    vka_cspace_make_path(&context->vka, irq, &irq_path);

    error = simple_get_IRQ_handler(&context->simple, irqN, irq_path);
    assert(error == 0);

    vka_object_t irq_aep_obj = { 0 };

    error = vka_alloc_notification(&context->vka, &irq_aep_obj);
    assert(error == 0);

    seL4_CPtr irq_aep = irq_aep_obj.cptr;
    error = seL4_IRQHandler_SetNotification(irq_path.capPtr, irq_aep);
    assert(error == 0);

    _irqaep = irq_aep;
    _irq = irq_path.capPtr;
    _dev = dev;

    KThreadInit(&_irqThread);
    _irqThread.name = tName;
    _irqThread.mainFunction = _irqMain;
    KThreadRun(&_irqThread, 254, NULL);
    return 0;
}