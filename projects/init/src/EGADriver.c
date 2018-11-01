#include "EGADriver.h"
#include <sel4platsupport/io.h>


#define EGA_TEXT_FB_BASE 0xB8000
#define MODE_WIDTH 80
#define MODE_HEIGHT 25


static ps_io_ops_t io_ops;


static void* mapVideoRam(InitContext *context) 
{
     void* vram = ps_io_map(&io_ops.io_mapper, EGA_TEXT_FB_BASE,
                                0x1000, false, PS_MEM_NORMAL);
    assert(vram != NULL);



    
    return vram;
}


static void writeVideoRam(uint16_t* vram, int row) 
{
    printf("VRAM mapped at: 0x%x\n", (unsigned int) vram);

    const int width = MODE_WIDTH;
    for (int col = 0; col < 80; col++) 
    {
        vram[width * row + col] =  ('0' + col) | (col << 8);
    }
}

int InitEGADriver(InitContext *context)
{

	int error = sel4platsupport_new_io_ops( context->vspace, context->vka, &io_ops);
    	assert(error == 0);

	void* ptr = mapVideoRam(context);

	if(ptr)
	{
	    for(int i=0;i<MODE_HEIGHT;i++)
    	    {
        	writeVideoRam((uint16_t*)ptr, i);
    	    }

	}
	return ptr != NULL;
}
