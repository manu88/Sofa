#include "PCI.h"
#include "Log.h"
#include "IODriver.h"
#include "DeviceTree.h"
#include "Environ.h"
#include <pci/pci.h>
 

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