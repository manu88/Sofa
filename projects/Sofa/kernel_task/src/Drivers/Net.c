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

IODeviceOperations _netDevOps = 
{
    .handleIRQ = _handleIRQ,
    .read = NULL,
    .write = NULL
};

static const char _devName[] = "Virtio-net-pci";


static NetworkDriver _driver = {0};
lwip_iface_t lwip_driver;

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


static void handle_irq(void *state, int irq_num)
{
    ethif_lwip_handle_irq(state, irq_num);
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

    _driver.driver =  ethif_new_lwip_driver_no_malloc(env->ops, NULL, native_ethdriver_init, &cfg, &lwip_driver);
    if(_driver.driver == NULL)
    {
        KLOG_INFO("ERROR unable to create lwip driver\n");
        free(dev);
        return NULL;
    }

    _driver.init_fn = ethif_get_ethif_init(&lwip_driver);
    _driver.handle_irq_fn = handle_irq;
    IRQServerRegisterIRQ(dev, 11);
    return dev;

    ip_addr_t addr, gw, mask;

    //FIXME: THis needs to be dynamic
    ipaddr_aton("192.168.0.1",   &gw);
    ipaddr_aton("10.0.2.15",     &addr);
    ipaddr_aton("255.255.255.*", &mask);

    assert(lwip_driver.netif == NULL);
    lwip_driver.netif = malloc(sizeof(struct netif));
    assert(lwip_driver.netif);

    netif_add(lwip_driver.netif, &addr, &mask, &gw, _driver.driver, _driver.init_fn, ethernet_input);
    assert(lwip_driver.netif);
    netif_set_up(lwip_driver.netif);
    netif_set_default(lwip_driver.netif);


    
    return dev;

}

static void _handleIRQ(IODevice* dev, int irqN)
{
    _driver.handle_irq_fn(_driver.driver, irqN);
}
