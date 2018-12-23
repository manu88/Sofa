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

#include "Usb.h"
#include <usb/usb.h>
#include <usb/plat/usb.h>
#include <stdio.h>
#include <assert.h>

typedef struct 
{
	ps_mutex_ops_t _mutexOps;
	usb_t _usbHost;

} USBContext;


static void* _MutexNew(void);
static int   _MutexLock(void *m);
static int   _MutexUnlock(void *m);
static int   _MutexDestroy(void *m);

static USBContext _context = 
{
	{
		&_context, // cookie
		_MutexNew,
		_MutexLock,
		_MutexUnlock,
		_MutexDestroy
	}
};



int USBInit(KernelTaskContext* context)
{
	printf("Init USB Driver\n");

	int error = usb_init(USB_HOST1 /*USB_HOST_DEFAULT*/ , &context->ops , &_context._mutexOps , &_context._usbHost);

	printf("usb_init ok \n");
	return error;
}


static void* _MutexNew()
{
	return &_context;
}

static int   _MutexLock(void *m)
{
	assert(m == &_context);
	return 0;
}

static int   _MutexUnlock(void *m)
{
	assert(m == &_context); 
	return 0;
}

static int   _MutexDestroy(void *m)
{
	assert(m == &_context); 
	return 0;
}
