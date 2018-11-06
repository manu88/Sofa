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


#include <string.h>
#include "Terminal.h"
#include "../Drivers/EGADriver.h"


static ssize_t ConsoleWrite (struct _inode *node,  const char*buffer ,size_t size);
static Inode* ConsoleOpen (struct _DeviceOperations * device, int flags );
static ssize_t ConsoleRead (struct _inode * node, char* buffer  , size_t size);


static int HandleKeyboardIRQ ( IOBaseDevice *device, int irqNum);

int TerminalInit( InitContext* context,const cspacepath_t* notificationSrc,  Terminal* terminal)
{
    memset(terminal, 0, sizeof(Terminal));
    
    terminal->devOps.OpenDevice 	= ConsoleOpen;
    terminal->devOps.fileOps.Write      = ConsoleWrite;
    terminal->devOps.fileOps.Read  	= ConsoleRead;


    // init keyboard
    int error = 0;
    error = !KeyboardDeviceInit( context, notificationSrc , &terminal->keyboard);
    //ZF_LOGF_IFERR(error, "Unable to initialize Keyboard .\n");

    if (error != 0)
    {
	return 0;
    }


    terminal->keyboard.super.HandleIRQ = HandleKeyboardIRQ;

    if( InitEGADriver( context) == 0)
    {
	return 0;
    }
    
//    ZF_LOGF_IFERR(error, "Failed to  init EGA driver\n");

    return  1;
}

static int HandleKeyboardIRQ ( IOBaseDevice *device, int irqNum)
{
    Terminal* term = (Terminal*) device;
    assert(term);

    KeyboardDevice* dev = &term->keyboard;

    for (;;) 
    {
        //int c = __arch_getchar();
        int c = ps_cdev_getchar(&dev->dev);
        if (c == EOF) {
            //read till we get EOF
            break;
        }
//        printf("You typed [%c]\n", c);

	terminal_putentryat(c ,VGA_COLOR_GREEN ,  0 , 1);
    }

    UNUSED int err = seL4_IRQHandler_Ack(dev->handler.capPtr);
    assert(err == 0);

    return 1;
}


static ssize_t ConsoleWrite (struct _inode *node,  const char*buffer ,size_t size)
{
    for(int i =0;i<size;i++)
    {
	terminal_putentryat(buffer[i] ,VGA_COLOR_RED ,  i , 0);
    }
    return (ssize_t)size;
}


static Inode* ConsoleOpen (struct _DeviceOperations * device, int flags )
{
    printf("Console Open request\n");

    Inode* node = malloc(sizeof(Inode) );
    node->operations = &device->fileOps;
    return node;
}


static ssize_t ConsoleRead (struct _inode * node, char* buffer  , size_t size)
{
    printf("Console read req\n");

    return 0;
}
