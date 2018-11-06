#include "EGADriver.h"
#include <sel4platsupport/io.h>


#define EGA_TEXT_FB_BASE 0xB8000
#define MODE_WIDTH 80
#define MODE_HEIGHT 25


/* Hardware text mode color constants. */
enum vga_color {
        VGA_COLOR_BLACK = 0,
        VGA_COLOR_BLUE = 1,
        VGA_COLOR_GREEN = 2,
        VGA_COLOR_CYAN = 3,
        VGA_COLOR_RED = 4,
        VGA_COLOR_MAGENTA = 5,
        VGA_COLOR_BROWN = 6,
        VGA_COLOR_LIGHT_GREY = 7,
        VGA_COLOR_DARK_GREY = 8,
        VGA_COLOR_LIGHT_BLUE = 9,
        VGA_COLOR_LIGHT_GREEN = 10,
        VGA_COLOR_LIGHT_CYAN = 11,
        VGA_COLOR_LIGHT_RED = 12,
        VGA_COLOR_LIGHT_MAGENTA = 13,
        VGA_COLOR_LIGHT_BROWN = 14,
        VGA_COLOR_WHITE = 15,
};


typedef struct
{
	DeviceOperations _devOps;

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

static void* mapVideoRam(InitContext *context) 
{
     void* vram = ps_io_map(&_egaContext.io_ops.io_mapper, EGA_TEXT_FB_BASE,
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
        vram[width * row + col] =  ('0' + col) | (2 << 8);
    }
}

int InitEGADriver(InitContext *context)
{
	memset(&_egaContext , 0 , sizeof(_EGAContext) );

	_egaContext._devOps.OpenDevice = ConsoleOpen;
	_egaContext._devOps.fileOps.Write      = ConsoleWrite;

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


DeviceOperations* EGADriverGetDeviceOps()
{
	return &_egaContext._devOps;
}
