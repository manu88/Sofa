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

#include "EGADriver.h"
#include <sel4platsupport/io.h>


#define EGA_TEXT_FB_BASE 0xB8000
#define MODE_WIDTH 80
#define MODE_HEIGHT 25



typedef struct
{
	ps_io_ops_t io_ops;

	uint16_t* videoPtr; 
} _EGAContext;


static _EGAContext _egaContext;


static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
        return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
        const size_t index = y * MODE_WIDTH + x;
        _egaContext.videoPtr[index] = vga_entry(c, color);
}

void terminalClear()
{
    const int width = MODE_WIDTH;
   
    for(int row=0;row<MODE_HEIGHT;row++)
    {
        for (int col = 0; col < 80; col++) 
        {
            _egaContext.videoPtr[width * row + col] = 0;
        }
    }
}

static void* mapVideoRam(InitContext *context) 
{
     void* vram = ps_io_map(&_egaContext.io_ops.io_mapper, EGA_TEXT_FB_BASE,
                                0x1000, false, PS_MEM_NORMAL);
    assert(vram != NULL);



    
    return vram;
}


static void writeVideoRam(uint16_t* vram, int row) 
{

    const int width = MODE_WIDTH;
    for (int col = 0; col < 80; col++) 
    {
        vram[width * row + col] =  ('0' + col) | (2 << 8);
    }
}

int InitEGADriver(InitContext *context)
{
	memset(&_egaContext , 0 , sizeof(_EGAContext) );

	int error = sel4platsupport_new_io_ops( context->vspace, context->vka, &_egaContext.io_ops);
    	assert(error == 0);

	_egaContext.videoPtr = mapVideoRam(context);
	
	/*
	if(_egaContext.videoPtr)
	{
	    for(int i=0;i<MODE_HEIGHT;i++)
    	    {
        	writeVideoRam((uint16_t*)_egaContext.videoPtr, i);
    	    }

	}
	*/
	terminalClear();
	return _egaContext.videoPtr != NULL;
}


