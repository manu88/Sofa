#include <ethdrivers/lwip.h>
#include <ethdrivers/intel.h>
#include <ethdrivers/virtio_pci.h>
#include <netif/etharp.h>
#include <lwip/udp.h>
#include "Environ.h"
#include "Net.h"
#include "KThread.h"
#include "DeviceTree.h"

IODevice _netDevice = IODeviceInit("Virtio-net-pci", IODevice_Net, NULL);

static KThread netThread;
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



static void
handle_irq(void *state, int irq_num)
{
    ethif_lwip_handle_irq(state, irq_num);
}

static int threadStart(KThread* thread, void *arg);

void NetInit(uint32_t iobase0)
{

    LWIP_DEBUG_ENABLED(LWIP_DBG_LEVEL_ALL);
    KernelTaskContext* env = getKernelTaskContext();

    ethif_virtio_pci_config_t cfg = {0};
    cfg.io_base = iobase0;    

    _driver.driver =  ethif_new_lwip_driver_no_malloc(env->ops, NULL, native_ethdriver_init, &cfg, &lwip_driver);
    if(_driver.driver == NULL)
    {
        printf("ERROR unable to create lwip driver\n");
        return;
    }

    _driver.init_fn = ethif_get_ethif_init(&lwip_driver);
    _driver.handle_irq_fn = handle_irq;
    _driver.irq_num = 11;

/**/
    cspacepath_t irq_path = { 0 };
    seL4_CPtr irq;

    int error = vka_cspace_alloc(&env->vka, &irq);
    assert(error == 0);
    vka_cspace_make_path(&env->vka, irq, &irq_path);

    error = simple_get_IRQ_handler(&env->simple, _driver.irq_num, irq_path);
    assert(error == 0);

    vka_object_t irq_aep_obj = { 0 };

    error = vka_alloc_notification(&env->vka, &irq_aep_obj);
    assert(error == 0);

    seL4_CPtr irq_aep = irq_aep_obj.cptr;
    error = seL4_IRQHandler_SetNotification(irq_path.capPtr, irq_aep);
    assert(error == 0);

// add interface

    ip_addr_t addr, gw, mask;

    //FIXME: THis needs to be dynamic
    ipaddr_aton("192.168.0.1",   &gw);
    ipaddr_aton("10.0.2.15", &addr);
    ipaddr_aton("255.255.255.*", &mask);

    assert(lwip_driver.netif == NULL);
    lwip_driver.netif = malloc(sizeof(struct netif));
    assert(lwip_driver.netif);

    netif_add(lwip_driver.netif, &addr, &mask, &gw, _driver.driver, _driver.init_fn, ethernet_input);
    assert(lwip_driver.netif);
    netif_set_up(lwip_driver.netif);
    netif_set_default(lwip_driver.netif);

// New thread
    KThreadInit(&netThread);
    netThread.name = "virtio-pci";
    netThread.mainFunction = threadStart;

    seL4_CPtr *args = malloc(sizeof(seL4_CPtr)*2);
    args[0] = irq_aep;
    args[1] = irq;
    KThreadRun(&netThread, 254, args);

    DeviceTreeAddDevice(&_netDevice);

}

static int threadStart(KThread* thread, void *arg)
{
    seL4_CPtr *args = arg;
    seL4_CPtr irq_aep = args[0];
    seL4_CPtr irq =  args[1];
    free(args);
    
    while(1)
    {
	    seL4_Wait(irq_aep,NULL);
        seL4_IRQHandler_Ack(irq);
        _driver.handle_irq_fn(_driver.driver, _driver.irq_num);
    }
    return 0;
}
