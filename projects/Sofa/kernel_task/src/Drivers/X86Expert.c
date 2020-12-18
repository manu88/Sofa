#include "X86Expert.h"
#include "Log.h"
#include "Environ.h"
#include <pci/pci.h>
#include "Blk.h"

static int DeviceTreeX86Start(void);

int PlatformExpertInit()
{
    KLOG_INFO("PlatformExpertInit for X86_64\n");
    return DeviceTreeX86Start();
}

static int DeviceTreeX86Start()
{
    //ACPITest();

    KernelTaskContext* env = getKernelTaskContext();
    int error = 0;
    
    printf("#### PCI SCAN\n");
    libpci_scan(env->ops.io_port_ops);
    printf("#### PCI SCAN\n");

    printf("Got %u pci devices\n", libpci_num_devices);

//Storage virtio 
    libpci_device_t *virtioBlkDev = libpci_find_device(0x1af4, 0x1001);
    if(virtioBlkDev)
    {
        printf("Got Virtio Blk device '%s' from '%s' subsystem %i\n", virtioBlkDev->vendor_name, virtioBlkDev->device_name, virtioBlkDev->subsystem_id);

        if(virtioBlkDev->subsystem_id == 2)
        {
            libpci_device_iocfg_t iocfg;
            libpci_read_ioconfig(&iocfg, virtioBlkDev->bus, virtioBlkDev->dev, virtioBlkDev->fun);
            libpci_device_iocfg_debug_print(&iocfg, false);
            uint32_t iobase0 =  libpci_device_iocfg_get_baseaddr32(&iocfg, 0);

            BlkInit(iobase0);
            //virtio_blk_init(iobase0);
        }
    }

//NET virtio: vid 0x1af4 did 0x1000
    libpci_device_t *virtioNetDev = libpci_find_device(0x1af4, 0x1000);

    if(virtioNetDev)
    {
        printf("Got Virtio Net device '%s' from '%s' subsystem %i\n", virtioNetDev->vendor_name, virtioNetDev->device_name, virtioNetDev->subsystem_id);
        if(virtioNetDev->subsystem_id == 1) // network card
        {
            libpci_device_iocfg_t iocfg;
            libpci_read_ioconfig(&iocfg, virtioNetDev->bus, virtioNetDev->dev, virtioNetDev->fun);
            libpci_device_iocfg_debug_print(&iocfg, false);
            uint32_t iobase0 =  libpci_device_iocfg_get_baseaddr32(&iocfg, 0);
            printf("IOBASE0 is %X\n", iobase0);
            NetInit(iobase0);
        }
        else
        {
            printf("Virtio device subsystem is %i\n", virtioNetDev->subsystem_id);
        }
    }

    return 0;
    
}