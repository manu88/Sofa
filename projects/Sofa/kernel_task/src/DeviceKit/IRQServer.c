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
#include "IRQServer.h"
#include "Log.h"
#include "KThread.h"
#include "Environ.h"
#include "utlist.h"



static const char tName[] = "IRQServer";




typedef struct _IRQInstance 
{
    struct _IRQInstance *next;
    int irqN;

    IODevice* recipient;
    seL4_CPtr irqAckHandler;
} IRQInstance;


typedef struct 
{
    KThread _irqThread;
    vka_object_t irq_aep_obj;

    IRQInstance *irqInstances;
} IRQServer;

static IRQServer _server;



static int _irqMain(KThread* th, void* arg);

int IRQServerInit()
{
    memset(&_server, 0, sizeof(IRQServer));
    KThreadInit(&_server._irqThread);
    _server._irqThread.name = tName;
    _server._irqThread.mainFunction = _irqMain;


    KernelTaskContext* context = getKernelTaskContext();

    int error = vka_alloc_notification(&context->vka, &_server.irq_aep_obj);
    if(error != 0)
    {
        return error;
    }
    assert(error == 0);

    error = KThreadRun(&_server._irqThread, 254, &_server);
    return error;
}

static int _irqMain(KThread* th, void* arg)
{
    IRQServer* server = arg;
    assert(arg);
    KLOG_INFO("Starting IRQ Server\n");

    while (1)
    {   
        seL4_Word sender = 0;
        seL4_Wait(_server.irq_aep_obj.cptr, &sender);
        int irqN = sender;

        int once = 1;
        IRQInstance* i = NULL;
        LL_FOREACH(server->irqInstances, i)
        {
            if(i->irqN == irqN)
            {   
                if(once)
                {
                    seL4_IRQHandler_Ack(i->irqAckHandler);
                    once = 0;
                }
                i->recipient->ops->handleIRQ(i->recipient, irqN);

            }
        }

#if 0        
        IRQInstance* instance = (IRQInstance*) sender; 
        assert(instance);
        assert(instance->recipient);
        assert(instance->irqAckHandler);
        seL4_IRQHandler_Ack(instance->irqAckHandler);
        
        instance->recipient->ops->handleIRQ(instance->recipient, -1);
#endif
    }
    
}

int IRQServerRegisterIRQ(IODevice* dev, int irqN)
{
    KLOG_INFO("IODeviceRegisterIRQ %i for '%s'\n", irqN, dev->name);

    int createIrqHandler = 1;
    IRQInstance* i = NULL;
    LL_FOREACH(_server.irqInstances, i)
    {
        if(i->irqN == irqN)
        {
            KLOG_INFO("IRQ %i already registered\n", irqN);
            createIrqHandler = 0;
            break;
        }
    }



    IRQInstance* instance = malloc(sizeof(IRQInstance));
    assert(instance);
    memset(instance, 0, sizeof(IRQInstance));
    instance->recipient = dev;
    instance->irqN = irqN;
    LL_APPEND(_server.irqInstances, instance);

    if(createIrqHandler)
    {
        KernelTaskContext* context = getKernelTaskContext();

        cspacepath_t irq_handler_path = { 0 };
        seL4_CPtr irq_handler;


        int error = vka_cspace_alloc(&context->vka, &irq_handler);
        assert(error == 0);
        vka_cspace_make_path(&context->vka, irq_handler, &irq_handler_path);

        error = simple_get_IRQ_handler(&context->simple, irqN, irq_handler_path);
        assert(error == 0);

        seL4_CPtr capMint = get_free_slot(&context->vka);
        error = cnode_mint(&context->vka, _server.irq_aep_obj.cptr, capMint, seL4_AllRights, (seL4_Word) irqN);
        assert(error == 0);


        error = seL4_IRQHandler_SetNotification(irq_handler_path.capPtr, capMint);
        assert(error == 0);

        instance->irqAckHandler = irq_handler_path.capPtr; 

    }

    size_t numIrqInstance = 0;
    IRQInstance* t = NULL;
    LL_COUNT(_server.irqInstances, t, numIrqInstance);
    KLOG_DEBUG("[IRQService] Got %zi IRQ instances\n", numIrqInstance); 
    return 0;
}