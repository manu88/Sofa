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
#include "IODevice.h"
#include "devFS.h"
#include <sel4platsupport/arch/io.h>
#include <utils/circular_buffer.h>

static char _circ_buffer[sizeof(circ_buf_t) + SERIAL_CIRCULAR_BUFFER_SIZE -1]; // '-1' 'cause 1 byte is already present in circ_buf_t

static circ_buf_t* getCircularBuffer()
{
    return (circ_buf_t *) _circ_buffer;
}

size_t ComGetAvailableChar()
{
    return getCircularBuffer()->tail - getCircularBuffer()->head;
}

static int consRead(ThreadBase* caller, File *file, void *buf, size_t numBytes);
static int consWrite(File *file, const void *buf, size_t numBytes);

static int consRead2(ThreadBase* caller, File *file, void *buf, size_t numBytes);
static int consWrite2(File *file, const void *buf, size_t numBytes);

static FileOps _consoleOps = 
{
    .asyncRead = 1,
    .Read = consRead,
    .Write = consWrite
};


static FileOps _consoleOps2 = 
{
    .asyncRead = 1,
    .Read = consRead2,
    .Write = consWrite2
};

static void onIrq(IODevice* dev, int irqN);

static IODeviceOperations _comDevOps = 
{
    .handleIRQ = onIrq
};

FileOps* getConsOps()
{
    return &_consoleOps;
}

FileOps* getConsOps2()
{
    return &_consoleOps2;
}

typedef struct 
{
    OnBytesAvailable waiter;

    size_t size;
    char until;
    void* ptr;
    void* ptr2;

} SerialWaiter;

static SerialWaiter _waiter = {0};

void ComHandleInput(IODevice* dev)
{
    ps_chardevice_t* charDev = (ps_chardevice_t*) dev->impl;

    int data = 0;
    while(data != EOF)
    {  
        data = ps_cdev_getchar(charDev);
        if(data > 0)
        {
#if 0
            if(data == '\03')
            {
                if(_controller.waitCtl)
                {
                    _controller.waitCtl(data, _controller.ptr);
                    continue;
                }
            }
#endif
            if(data == '\r')
                data = '\n';

            circ_buf_put(getCircularBuffer(), data);

            if(_waiter.waiter &&  _waiter.size)
            {
                // echo only if someone is waiting on us!
                //putchar(data);
                ps_cdev_putchar(charDev, data);
                //fflush(stdout);
                //
                if(_waiter.until && data == _waiter.until)
                {
                    _waiter.waiter(ComGetAvailableChar(), _waiter.until, _waiter.ptr, _waiter.ptr2);
                    memset(&_waiter, 0, sizeof(_waiter));
                }
                else if(ComGetAvailableChar() >= _waiter.size)
                {
                    _waiter.waiter(ComGetAvailableChar(), (char) 0, _waiter.ptr, _waiter.ptr2);
                    memset(&_waiter, 0, sizeof(_waiter));
                }
            }

        }
    }
}

static void onIrq(IODevice* dev, int irqN)
{
    ps_chardevice_t* charDev = (ps_chardevice_t*) dev->impl;
    assert(charDev); 
    ps_cdev_handle_irq(charDev, 0);
    ComHandleInput(dev);
}

size_t ComCopyAvailableChar(char* dest, size_t maxSize)
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

static void onBytesAvailable(size_t size, char until, void* ptr, void* buf)
{
    ThreadBase* caller = (ThreadBase*) ptr;
    assert(caller);

    size_t bytes = ComCopyAvailableChar(buf, size);
    ((char*)buf)[bytes] = 0;

    seL4_MessageInfo_t msg = seL4_MessageInfo_new(0, 0, 0, 3);
    seL4_SetMR(1, 0);
    seL4_SetMR(2, bytes);            
    
    seL4_Send(caller->replyCap, msg);
    cnode_delete(&getKernelTaskContext()->vka, caller->replyCap);
    caller->replyCap = 0;
    caller->state = ThreadState_Running;
}


int ComRegisterWaiter(OnBytesAvailable callback, size_t forSize, char until, void* ptr, void* ptr2)
{
    _waiter.waiter = callback;
    _waiter.size = forSize;
    _waiter.ptr = ptr;
    _waiter.ptr2 = ptr2;
    _waiter.until = until;
    return 0;
}

static int consRead(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    KernelTaskContext* env = getKernelTaskContext();

    assert(caller->replyCap == 0);

    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        KLOG_TRACE("Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        return -ENOMEM;
    }

    caller->replyCap = slot;
    caller->currentSyscallID = 0;
    ComRegisterWaiter(onBytesAvailable, numBytes, '\n', caller, buf);
    return -1;
}

static int consWrite(File *file, const void *buf, size_t numBytes)
{
    assert(file->impl);
    IODevice* dev = file->impl;
    ps_chardevice_t* charDev = (ps_chardevice_t*) dev->impl;
    assert(charDev);

    char* b = buf; 
    for(size_t i=0;i<numBytes;i++)
    {
        int v = b[i];
        ps_cdev_putchar(charDev, v);
    }
    return numBytes;
}


static int consRead2(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    assert(0);
}

static int consWrite2(File *file, const void *buf, size_t numBytes)
{
    assert(0);
}




static int DevProducesIRQ(const ps_chardevice_t* d)
{
    for (int i=0;i<256;i++)
    {
        if(ps_cdev_produces_irq(d, i))
        {
            return i;
        }
    }
    return -1;
}

void AddComDev(IODriver *drv, IONode * n)
{
    KernelTaskContext* context = getKernelTaskContext();

    IODevice* com = malloc(sizeof(IODevice));
    if(!com)
    {
        KLOG_ERROR("PCI._AddComDev: unable to alloc space for device '%s'\n", n->name);
        return;
    }
    IODeviceInit(com, n->name, IODevice_CharDev);
    DeviceTreeAddDevice(com);

    n->driver = drv;
    ps_chardevice_t* comDev = malloc(sizeof(ps_chardevice_t));
    assert(comDev);

    int comID = n->name[3] - '0';
    com->ops = &_comDevOps;
    com->impl = ps_cdev_init(comID -1 , &context->ops , comDev);
    assert(com->impl);

    int irqN = DevProducesIRQ(comDev);
    KLOG_DEBUG("PCI Driver '%s' produces IRQ %i\n", n->name, irqN);

    if(irqN > 0)
    {
        IODeviceRegisterIRQ(com, irqN);
    }

// create a file in /dev
    DevFile* comFile = malloc(sizeof(DevFile));
    if(comFile)
    {
        memset(comFile, 0, sizeof(DevFile));
        comFile->name = strdup(n->name);
        sprintf(comFile->name, "tty%i", comID-1);
        if(strcmp(n->name, "COM1") == 0)
        {
            circ_buf_init(SERIAL_CIRCULAR_BUFFER_SIZE, getCircularBuffer());
            comFile->ops = getConsOps();
        }
        else
        {
            comFile->ops = getConsOps2();
        }
        comFile->device = com;
        DevFSAddDev(comFile);
        n->devFile = comFile;
    }
}