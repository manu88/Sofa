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
            IODevice* com = malloc(sizeof(IODevice));
            if(!com)
            {
                KLOG_ERROR("PCI._checkISA: unable to alloc space for device '%s'\n", n->name);
            }
            IODeviceInit(com, n->name, IODevice_CharDev);
            DeviceTreeAddDevice(com);

            n->driver = (IODriver*) &_pciDriver;
        }
    }
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
        NetInit(iobase0);
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
    
    n->driver = (IODriver*) &_pciDriver;
    return 0;
}