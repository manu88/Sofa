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
#include "Vesa.h"
#include "Log.h"
#include "Environ.h"
#include "ProcessList.h"
#include <platsupport/io.h>
#include <stdlib.h>
#include <math.h>


typedef struct 
{
    IODevice dev;
    const seL4_X86_BootInfo_VBE* info;

    void* fb;
} VesaDevice;


static long VesaMapMemory(IODevice* dev, Thread* caller, int code);

static IODeviceOperations _vesaOps = 
{
    .read = NULL,
    .write = NULL,
    .handleIRQ = NULL,
    .regIface = NULL,
    .mapMemory = VesaMapMemory
};


static void putPixel(VesaDevice* dev, int x, int y, uint32_t color)
{
    assert(dev->fb != NULL);

    unsigned int coord_factor = 4;
    size_t len = 3;
    char* target = ((char *)dev->fb) + (y * 640 + x) * coord_factor;

    target[0] = color & 255;
    target[1] = (color >> 8) & 255;
    target[2] = (color >> 16) & 255;
}

static void DisplayTestPic(VesaDevice* dev)
{

    for(int i=0;i<100;i++)
    {
        putPixel(dev, i,i, 0X0000FF);    
    }
    putPixel(dev, 0,0, 0X00FFFF);
    putPixel(dev, 320,240, 0X00FFFF);
    putPixel(dev, 0,240, 0X00FFFF);

    for(int i=0;i<100;i++)
    {
        putPixel(dev, 639-i,479-i, 0X00FF00);    
    }
}

static void VesaMapVideoRam(VesaDevice* dev, ps_io_mapper_t *io_mapper)
{    
    size_t size = dev->info->vbeModeInfoBlock.vbe12_part1.yRes * dev->info->vbeModeInfoBlock.vbe30.linBytesPerScanLine;
    KLOG_DEBUG("Vesa framebuffer size is %zi\n", size);
    dev->fb = (void*) ps_io_map(io_mapper,
            dev->info->vbeModeInfoBlock.vbe20.physBasePtr,
            size,
            0,
            PS_MEM_HW);
    assert(dev->fb != NULL);
}


IODevice* VesaInit(const seL4_X86_BootInfo_VBE* info)
{
    VesaDevice* dev = malloc(sizeof(VesaDevice));
    memset(dev, 0, sizeof(VesaDevice));
    if(dev)
    {
        IODeviceInit((IODevice*) dev, "Framebuffer", IODevice_FrameBuffer, &_vesaOps);
        dev->info = info;
        KLOG_DEBUG("xRes=%i yRes=%i linBytesPerScanLine=%i bpp=%u\n", 
                   info->vbeModeInfoBlock.vbe12_part1.xRes,
                   info->vbeModeInfoBlock.vbe12_part1.yRes,
                   info->vbeModeInfoBlock.vbe30.linBytesPerScanLine,
                   info->vbeModeInfoBlock.vbe12_part1.bitsPerPixel
                   );

        VesaMapVideoRam(dev, &getKernelTaskContext()->io_mapper);
        DisplayTestPic(dev);

    }
    return (IODevice*)dev;
}

static long VesaMapMemory(IODevice* _dev, Thread* caller, int code)
{
    VesaDevice* dev = (VesaDevice*) _dev;


    size_t size = dev->info->vbeModeInfoBlock.vbe12_part1.yRes * dev->info->vbeModeInfoBlock.vbe30.linBytesPerScanLine;

    size_t numPages = (size_t)ceil(size/4096);
    KLOG_DEBUG("VesaMapMemory request code=%i share %zi (%zi pages)\n", code, size, numPages);
    void * ret = vspace_share_mem(getMainVSpace(), &caller->_base.process->native.vspace, dev->fb, numPages, PAGE_BITS_4K, seL4_ReadWrite, 1);

    return ret;
}