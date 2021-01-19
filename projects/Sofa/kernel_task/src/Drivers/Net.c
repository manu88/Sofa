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
#include <ethdrivers/lwip.h>
#include <ethdrivers/intel.h>
#include <ethdrivers/virtio_pci.h>
#include <netif/etharp.h>
#include <lwip/udp.h>
#include "Environ.h"
#include "Net.h"
#include "Log.h"
#include "IRQServer.h"
#include "DeviceTree.h"


static void _handleIRQ(IODevice* dev, int irqN);
static int _RegisterIface(IODevice* dev, void* netInterface, const ip_addr_t* addr, const ip_addr_t*gw, const ip_addr_t* mask);

IODeviceOperations _netDevOps = 
{
    .handleIRQ = _handleIRQ,
    .read = NULL,
    .write = NULL,
    .regIface = _RegisterIface
};

static const char _devName[] = "Virtio-net-pci";

static int
native_ethdriver_init(
    struct eth_driver *eth_driver,
    ps_io_ops_t io_ops,
    void *config)
{
    ethif_virtio_pci_config_t *virtio_config = config;

    int error = ethif_virtio_pci_init(eth_driver, io_ops, config);

    return error;
}


IODevice* NetInit(uint32_t iobase0)
{
    IODevice* dev = malloc(sizeof(IODevice));
    if(!dev)
    {
        return NULL;
    }

    IODeviceInit(dev, _devName, IODevice_Net, &_netDevOps);

    LWIP_DEBUG_ENABLED(LWIP_DBG_LEVEL_ALL);
    KernelTaskContext* env = getKernelTaskContext();

    ethif_virtio_pci_config_t cfg = {0};
    cfg.io_base = iobase0;    

    lwip_iface_t *lWipIface = ethif_new_lwip_driver(env->ops, NULL, native_ethdriver_init, &cfg);

    if(lWipIface == NULL)
    {
        KLOG_INFO("ERROR unable to create lwip driver\n");
        free(dev);
        return NULL;
    }
    dev->impl = lWipIface;

    IRQServerRegisterIRQ(dev, 11);
    return dev;
}

static int _RegisterIface(IODevice* dev, void* netInterface, const ip_addr_t* addr, const ip_addr_t*gw, const ip_addr_t* mask)
{
    struct netif *ret = netif_add(netInterface, addr, mask, gw, dev->impl, ethif_get_ethif_init(dev->impl), ethernet_input);
    if(ret == NULL)
    {
        return -1;
    }
    return 0;
}

static void _handleIRQ(IODevice* dev, int irqN)
{
    lwip_iface_t *iface = dev->impl;
    if(!iface->netif)
    {
        KLOG_DEBUG("Net.HandleIRQ: iface for %s not setup!\n", dev->name);
        return;
    }
    ethif_lwip_handle_irq(iface, irqN);
}
