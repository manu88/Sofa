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
#include "PCI.h"
#include "Log.h"
#include "IODriver.h"
#include "DeviceTree.h"
#include "Environ.h"
#include <pci/pci.h>
#include "Blk.h"
#include "Net.h"
#include "Serial.h"
#include "BGADriver.h"

typedef struct _PCIDriver
{
    IODriver base;
    u_int8_t inQEMU;
} PCIDriver;

static PCIDriver _pciDriver;

static void _PoweroffQEMU(void* ptr)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    ps_io_port_out(&ctx->ops.io_port_ops, 0x604, 2, 0x2000);
}

static void _checkFWCF(PCIDriver* drv, IONode * fwcfNode)
{
    // see https://wiki.osdev.org/QEMU_fw_cfg
    char eisaID[9] = "";
    if(IONodeGetEISAID(fwcfNode, eisaID) == 0)
    {
        if(strcmp(eisaID, "QEMU0002") == 0)
        {
            KLOG_INFO("Running under QEMU\n");
            drv->inQEMU = 1;
            DeviceTreeAddPoweroff(_PoweroffQEMU, drv);
        }
    }
}



static void _checkISA(PCIDriver* drv, IONode * isaNode)
{
    IONode*n = NULL;
    IONodeForEachChildren(isaNode, n)
    {
        if(IONodeEISAIDIs(n, "PNP0501") == 0)
        {
            AddComDev((IODriver*) drv, n);
        }
    }
/*
    IONode* ega = malloc(sizeof(IONode));
    IONodeInit(ega, "COM6");
    IONodeAddChild(isaNode, ega);
    AddComDev((IODriver*) drv, ega);
*/
}

static void _scanPCIDevice(PCIDriver* driver, libpci_device_t *pciDev)
{
    KLOG_DEBUG("PCIDriver scan device %X %X '%s' '%s'\n",
               pciDev->vendor_id,
               pciDev->device_id,
               pciDev->vendor_name,
               pciDev->device_name);

    libpci_device_iocfg_t iocfg;
    libpci_read_ioconfig(&iocfg, pciDev->bus, pciDev->dev, pciDev->fun);
    uint32_t iobase0 =  libpci_device_iocfg_get_baseaddr32(&iocfg, 0);

    //Storage virtio vid=0x1af4, pid=0x1001
    if(pciDev->vendor_id == 0x1af4
       && pciDev->device_id==0x1001
       && pciDev->subsystem_id == 2)
    {
        KLOG_DEBUG("Got VIRTIO BLOCK DEVICE\n");
        BlkInit(iobase0);
    }
    //NET virtio: vid 0x1af4 did 0x1000
    else if(pciDev->vendor_id == 0x1af4
            && pciDev->device_id==0x1000
            && pciDev->subsystem_id == 1)
    {
        KLOG_DEBUG("Got VIRTIO NET DEVICE\n");

        IODevice* netDev = NetInit(iobase0);
        if(netDev)
        {
            DeviceTreeAddDevice(netDev);
        }
    }

}

static int _scanPCIBus(PCIDriver* driver)
{
    KernelTaskContext* env = getKernelTaskContext();
    libpci_scan(env->ops.io_port_ops);

    for(uint32_t i =0;i<libpci_num_devices;i++)
    {
        libpci_device_t *pciDev = libpci_device_list + i;
        _scanPCIDevice(driver, pciDev);
    }

}


int PCIDriverInit(IONode *n)
{
    KLOG_INFO("PCIDriverInit HID='%s'\n", n->name);
    memset(&_pciDriver, 0, sizeof(PCIDriver));

    IONode* fwConfig = IONodeGetChildren(n, "FWCF");
    if(fwConfig)
    {
        _checkFWCF(&_pciDriver, fwConfig);
    }

    IONode* isaNode = IONodeGetChildren(n, "ISA");
    if(isaNode)
    {
        KLOG_DEBUG("Enum ISA Node\n");
        _checkISA(&_pciDriver, isaNode);
    }

    _scanPCIBus(&_pciDriver);

    IONode* ega = malloc(sizeof(IONode));
    IONodeInit(ega, "EGA");
    IONodeAddChild(isaNode, ega);

    IODriver* bgaDev = BGAInit(ega, (void *)0x1000000);
    if(bgaDev)
    {
        ega->driver = bgaDev;
    }
    
    n->driver = (IODriver*) &_pciDriver;

#if 0
    void *bga_ptr = (void *)0x1000000;
    bga_p bga = bga_init(bga_ptr, out16, in16);
    bga_set_mode(bga, 1024, 768, 24);

    char purple[] = { 210, 101, 141 };
    char blue[] = { 197, 103, 0 };


#define CHAR_WIDTH 14
#define CHAR_HEIGHT 26
#define WIDTH 312
#define HEIGHT 300
    for (unsigned int i = 100; i < 100 + WIDTH; i++) {
        for (unsigned int j = 100; j < 100 + 4 + CHAR_HEIGHT; j++) {
            bga_set_pixel(bga, i, j, purple);
        }
        bga_set_pixel(bga, i, 100 + HEIGHT, purple);
    }
    for (unsigned int i = 100; i < 100 + HEIGHT; i++) {
        bga_set_pixel(bga, 100, i, purple);
        bga_set_pixel(bga, 100 + WIDTH, i, purple);
    }

    for (unsigned int i = 200 + WIDTH; i < 200 + 2 * WIDTH; i++) {
        for (unsigned int j = 100; j < 100 + 4 + CHAR_HEIGHT; j++) {
            bga_set_pixel(bga, i, j, blue);
        }
        bga_set_pixel(bga, i, 100 + HEIGHT, blue);
    }
    for (unsigned int i = 100; i < 100 + HEIGHT; i++) {
        bga_set_pixel(bga, 200 + WIDTH, i, blue);
        bga_set_pixel(bga, 200 + 2 * WIDTH, i, blue);
    }
#endif
    return 0;
}
