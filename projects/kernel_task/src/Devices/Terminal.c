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


#define MAX_CHAR_QUEUE 64

static ssize_t ConsoleWrite (struct _inode *node,  const char*buffer ,size_t size);
static int     ConsoleOpen (struct _inode * node, int flags );
static int     ConsoleClose (Inode * node);
static ssize_t ConsoleRead (struct _inode * node, char* buffer  , size_t size);


static int HandleKeyboardIRQ ( IOBaseDevice *device, int irqNum);

static INodeOperations termOps = {ConsoleOpen , ConsoleClose , NULL  };
static FileOperations  termFileOps = {ConsoleRead , ConsoleWrite, FileOperation_NoLseek };


static void terminal_putchar(Terminal* term , char c);

int TerminalInit( KernelTaskContext* context,const cspacepath_t* notificationSrc,  Terminal* terminal)
{
    memset(terminal, 0, sizeof(Terminal));
    
    if(InodeInit( &terminal->node,INodeType_File, "console") == 0)
    {
	return 0;
    }

    terminal->node.operations = &termFileOps;
    terminal->node.inodeOperations = &termOps;
//    terminal->devOps.userContext	= terminal;
//    terminal->devOps.OpenDevice 	= ConsoleOpen;
//    terminal->devOps.fileOps.Write      = ConsoleWrite;
//    terminal->devOps.fileOps.Read  	= ConsoleRead;

    terminal->node.userData = terminal;
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
    

    cqueue_init( &terminal->inputChar , MAX_CHAR_QUEUE);

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
        if (c == EOF) 
 	{
            //read till we get EOF
            break;
        }
	cqueue_push(&term->inputChar , (cqueue_item_t) c);

  //      printf("You typed [%c] size %zi\n", c ,  cqueue_size(&term->inputChar ) );

//	terminal_putentryat(c ,VGA_COLOR_GREEN ,  0 , 1);
	terminal_putchar(term , c);


    }

    UNUSED int err = seL4_IRQHandler_Ack(dev->handler.capPtr);
    assert(err == 0);

    return 1;
}

void terminal_clear(Terminal* term)
{
	for(int i=0;i<MODE_WIDTH;i++)
	{
		for(int j = 0;j<MODE_HEIGHT;j++)
		{
			terminal_putentryat(' ', VGA_COLOR_RED, term->terminal_column, term->terminal_row);
		}
	}
}


static void terminal_putchar(Terminal* term , char c) 
{
	if (c == '\n' || c=='\r')
	{
	     term->terminal_column = 0;
	     term->terminal_row +=1;
	}
	else 
	{
             terminal_putentryat(c, VGA_COLOR_RED, term->terminal_column, term->terminal_row);
	}

	

        if (++term->terminal_column == MODE_WIDTH) 
	{
                term->terminal_column = 0;
                if (++term->terminal_row == MODE_HEIGHT)
                        term->terminal_row = 0;
        }
}

void terminal_putString( Terminal* term, const char* str)
{
    for(int i =0;i<strlen(str);i++)
    {
        terminal_putchar(term,str[i]);
    }
}


static ssize_t ConsoleWrite (struct _inode *node,  const char*buffer ,size_t size)
{

    Terminal* term = node->userData;
    assert(term);

//0xA , 0x0 , 0xB 

    if (size == 3 && (uint8_t)buffer[0] == 0xA)
    {
	const uint8_t *cmd = (const uint8_t *) buffer;
	if (cmd[1] == 0x0 && cmd[2] == 0xB)
	{
		terminal_clear(term);
		term->terminal_column = 0;
		term->terminal_row    = 0;
		return 0;
	} 
    }

    for(int i =0;i<size;i++)
    {
	terminal_putchar(term,buffer[i]);
//	terminal_putentryat(buffer[i] ,VGA_COLOR_RED ,  i , 0);
    }
    return (ssize_t)size;
}


static int     ConsoleClose (Inode * node)
{
	return 0;
}

static int ConsoleOpen (struct _inode *node, int flags )
{
    Terminal* term = (Terminal*)node->userData;
    assert(term);

/*
    Inode* node = malloc(sizeof(Inode) );
    node->operations = &device->fileOps;
    node->userData   = term;
    return node;
*/
    return 0;
}


static ssize_t ConsoleRead (struct _inode * node, char* buffer  , size_t size)
{
    Terminal* term = (Terminal*)node->userData;
    assert(term);


    size_t realReadSize = size < cqueue_size(&term->inputChar ) ? size : cqueue_size(&term->inputChar );

//    printf("Console read req size %li |  buffer size %li | real %li\n" ,size, cqueue_size(&term->inputChar ) , realReadSize);


    for( int i=0;i <realReadSize; i++)
    {
	char c = (char) cqueue_pop(&term->inputChar);
        buffer[i] = c;
    }
    /*
    if(cqueue_size(&term->inputChar ))
    {
	char c = (char) cqueue_pop(&term->inputChar);
	buffer[0] = c;

	return 1;
    }
*/
    return realReadSize;
}