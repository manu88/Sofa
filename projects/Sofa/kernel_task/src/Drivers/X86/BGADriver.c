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
#include "BGADriver.h"
#include "DeviceTree.h"
#include "IODriver.h"
#include "bga.h"
#include "Environ.h"
#include "Log.h"


IODriver drv;

static ssize_t _Read(IODevice* dev, size_t sector, char* buf, size_t bufSize);
static ssize_t _Write(IODevice* dev, size_t sector, const char* buf, size_t bufSize);

IODeviceOperations _bgaOps = 
{
    .read = _Read,
    .write = _Write
};

typedef struct 
{
    IODevice dev;
    bga_p bga;
}BGADevice;

/* Callbacks used below. */
static void out16(uint16_t port, uint16_t value)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    ps_io_port_out(&ctx->ops.io_port_ops, port, 2, value);
}

static uint16_t in16(uint16_t port)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    uint32_t result = 0;
    int error = ps_io_port_in(&ctx->ops.io_port_ops, port, 2, &result);
    if (error) {
        return 0;
    }
    return (uint16_t) result;
}

IODriver* BGAInit(void* fbuf)
{

    BGADevice* dev = malloc(sizeof(BGADevice));
    IODeviceInit((IODevice*) dev, "bga", IODevice_FrameBuffer, &_bgaOps);

    dev->bga = bga_init(fbuf, out16, in16);

    uint16_t version = bga_version(dev->bga);
    KLOG_DEBUG("BGA version is %X\n", version);
    DeviceTreeAddDevice((IODevice*) dev);

    return &drv;
}


static ssize_t _Read(IODevice* dev, size_t index, char* buf, size_t bufSize)
{
    KLOG_DEBUG("BGADevice Read request index %zi data size %zi\n", index, bufSize);
    return bufSize;
}

static int a = 0;
static ssize_t _Write(IODevice* _dev, size_t index, const char* buf, size_t bufSize)
{

    BGADevice* dev = (BGADevice*) _dev; 
    KLOG_DEBUG("BGADevice Write request index %zi data size %zi\n", index, bufSize);

    if(a == 0)
    {
        //bga_set_mode(dev->bga, 1024, 768, 24);
        bga_set_mode(dev->bga, 640, 480, 8);
    }
    else
    {     
        char *fb = bga_get_framebuffer(dev->bga);
        fb[a] = 255;
/*      
        char purple[] = { 210, 101, 141 };

        for(int i=0;i<50;i++)
        {
            bga_set_pixel(dev->bga, 100+i , 100+i, purple);
        }
*/
    }
    a++;
    return bufSize;
}