#include <ethdrivers/lwip.h>
#include <ethdrivers/intel.h>
#include <ethdrivers/virtio_pci.h>
#include <netif/etharp.h>
#include "Environ.h"
#include "Net.h"



static int
native_ethdriver_init(
    struct eth_driver *eth_driver,
    ps_io_ops_t io_ops,
    void *config)
{
    ethif_virtio_pci_config_t *virtio_config = config;
    printf("Call EthDriver Init iobase 0X%X mmio_base %p\n", virtio_config->io_base, virtio_config->mmio_base);
//    virtio_config->io_base = 0XC000;
//    ps_io_ops_t *sys_io_ops = (ps_io_ops_t*) config;
    int error;
    error = ethif_virtio_pci_init(eth_driver, io_ops, config);
//    error = ethif_imx6_init(eth_driver, *sys_io_ops, NULL);
    printf("Call EthDriver Init returned %i\n", error);
    return error;
}

static void
handle_irq(void *state, int irq_num)
{
    ethif_lwip_handle_irq(state, irq_num);
    printf("after ethif_lwip_handle_irq\n");
}



static NetworkDriver _driver = {0};
lwip_iface_t lwip_driver;

void NetInit(uint32_t iobase0)
{
    LWIP_DEBUG_ENABLED(LWIP_DBG_MIN_LEVEL);
    KernelTaskContext* env = getKernelTaskContext();
    printf("---->NetInit\n");

    ethif_virtio_pci_config_t cfg = {0};
    cfg.io_base = iobase0;    
    //cfg.mmio_base = 0xfe000000;
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

#if 1

    ip_addr_t addr, gw, mask;
    ipaddr_aton("192.168.0.1",   &gw);
    ipaddr_aton("192.168.0.100", &addr);
    ipaddr_aton("255.255.255.°", &mask);

    assert(lwip_driver.netif == NULL);
    lwip_driver.netif = malloc(sizeof(struct netif));
    assert(lwip_driver.netif);
    struct netif * ret = netif_add(lwip_driver.netif, &addr, &mask, &gw, _driver.driver, _driver.init_fn, ethernet_input);
    assert(ret);

#endif
    seL4_MessageInfo_t msg = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_Send(irq_aep, msg);
    printf("Begin irq loop\n");
    while (1)
    {
        seL4_Wait(irq_aep,NULL);
        printf("IRQ\n");
         _driver.handle_irq_fn(_driver.driver, _driver.irq_num);
        seL4_IRQHandler_Ack(irq);
    }
    

    printf("<----NetInit\n");
}