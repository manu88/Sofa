/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include "Keyboard.h"
#include <vka/object_capops.h> // vka_mint_object
#include "../MainLoop.h"


static void set_devEp(KeyboardDevice* dev);

static int KeyboardInit(IOBaseDevice *device)
{
	printf("Keyboard is initialized\n");
	return 1;
}

static int KeyboardDeInit (IOBaseDevice *device)
{
    return 1;
}


int KeyboardDeviceInit(InitContext* context, const cspacepath_t* notificationSrc, KeyboardDevice* keyboard)
{
	if(IOBaseDeviceInit( &keyboard->super) == 0 )
	{
		return 0;
	}

        keyboard->super.InitDevice = KeyboardInit;
	keyboard->super.DeInitDevice = KeyboardDeInit;
	int error = 0;

	keyboard->context = context;

	error = vka_cspace_alloc_path(&context->vka, &keyboard->ep);
        assert(error == 0);

	if (error != 0)
	{
		return 0;
	}

	error = vka_cnode_mint(&keyboard->ep, notificationSrc, seL4_AllRights, IRQ_BADGE_KEYBOARD | IRQ_EP_BADGE );
        assert(error == 0); 
 

	ps_chardevice_t *ret;
        ret = ps_cdev_init(PC99_KEYBOARD_PS2, &context->opsIO, &keyboard->dev);
        assert(ret != NULL);
	
	if (ret == NULL)
	{
		return 0;
	}

	set_devEp(keyboard);

	return 1;
}

// creates IRQHandler cap "handler" for IRQ "irq"
static void
get_irqhandler_cap( KeyboardDevice* keyboard ,int irq, cspacepath_t* handler)
{
    seL4_CPtr cap;
    // get a cspace slot
    UNUSED int err = vka_cspace_alloc(&keyboard->context->vka, &cap);
    assert(err == 0);

    // convert allocated cptr to a cspacepath, for use in
    // operations such as Untyped_Retype
    vka_cspace_make_path(&keyboard->context->vka, cap, handler);

    // exec seL4_IRQControl_Get(seL4_CapIRQControl, irq, ...)
    // to get an IRQHandler cap for IRQ "irq"
    err = simple_get_IRQ_handler(&keyboard->context->simple, irq, *handler);
    assert(err == 0);
}

// hook up endpoint (dev->ep) with IRQ of char device (dev->dev)
static void set_devEp(KeyboardDevice* dev) 
{
    // Loop through all IRQs and get the one device needs to listen to
    // We currently assume there it only needs one IRQ.
    int irq;
    for (irq = 0; irq < 256; irq++) {
        if (ps_cdev_produces_irq(&dev->dev, irq)) {
            break;
        }
    }
    printf ("irq=%d\n", irq);

    //create IRQHandler cap
    get_irqhandler_cap( dev,irq, &dev->handler);

    /* Assign AEP to the IRQ handler. */
    UNUSED int err = seL4_IRQHandler_SetNotification(
            dev->handler.capPtr, dev->ep.capPtr);
    assert(err == 0);

    //read once: not sure why, but it does not work without it, it seems
    ps_cdev_getchar(&dev->dev);
    err = seL4_IRQHandler_Ack(dev->handler.capPtr);
    assert(err == 0);
}
