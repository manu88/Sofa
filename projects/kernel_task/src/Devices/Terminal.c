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

static const char consoleName[] = "console";


int TerminalInit( KernelTaskContext* context,const cspacepath_t* notificationSrc,  Terminal* terminal)
{
    memset(terminal, 0, sizeof(Terminal));
    
    if(InodeInit( &terminal->node,INodeType_File, consoleName) == 0)
    {
        return 0;
    }

    terminal->node.operations = &termFileOps;
    terminal->node.inodeOperations = &termOps;

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
     
    terminal->color = VGA_COLOR_WHITE;
    cqueue_init( &terminal->inputChar , MAX_CHAR_QUEUE);

    terminal->_escState = cansid_init();
    terminal->scroll = 1;
    
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
        else if (c < 0) // error
        {
            printf("ps_cdev_getchar returned error %i\n", c);
            break;
        }

        if (c == 0x03) // ctrl-c
        {
            printf("CTRL-C \n");

        }
        // special chars
        else if (c >= 0x11 && c <= 0x14)
        {
            printf("control char %x\n", c);
        }

        else
        {
            cqueue_push(&term->inputChar , (cqueue_item_t) c);

            terminal_putchar(term , c);
        }

    } // end for (;;)

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
			terminal_putentryat(' ', term->color, i, j);
		}
	}
	term->terminal_column = 0;
	term->terminal_row    = 0;
}


static void terminal_putchar(Terminal* term , char c) 
{
	if (c == '\n' || c=='\r')
	{
	     term->terminal_column = 0;
	     term->terminal_row +=1;

	     if(term->terminal_row == MODE_HEIGHT)
	     {
		 term->terminal_row -=1;
 	         terminal_scroll(1);
	     }
	}
	else 
	{
        terminal_putentryat(c, term->color, term->terminal_column, term->terminal_row);
	}

	

    if (++term->terminal_column == MODE_WIDTH)
	{
        term->terminal_column = 0;
        if (++term->terminal_row == MODE_HEIGHT)
        {
            term->terminal_row = 0;
        }
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

    for(int i =0;i<size;i++)
    {
        struct color_char ch = cansid_process(&term->_escState, buffer[i] );
        
        if (term->_escState.operation == CANSID_CLEAR)
        {
            terminal_clear(term);
            term->terminal_column = 0;
            term->terminal_row    = 0;
        }
        else if (term->_escState.operation == CANSID_COLOR)
        {
            term->color = term->_escState.values[0] - 30;
        }
        if (ch.ascii)
        {
            terminal_putchar(term,buffer[i]);
        }
    }
    
    return (ssize_t)size;
}


static int ConsoleClose (Inode * node)
{
	return 0;
}

static int ConsoleOpen (struct _inode *node, int flags )
{
    Terminal* term = (Terminal*)node->userData;
    assert(term);

    return 0;
}


static ssize_t ConsoleRead (struct _inode * node, char* buffer  , size_t size)
{
    Terminal* term = (Terminal*)node->userData;
    assert(term);


    size_t realReadSize = size < cqueue_size(&term->inputChar ) ? size : cqueue_size(&term->inputChar );

    for( int i=0;i <realReadSize; i++)
    {
        char c = (char) cqueue_pop(&term->inputChar);
        buffer[i] = c;
    }
    
    return realReadSize;
}

