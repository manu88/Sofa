#include <ethdrivers/lwip.h>
#include <ethdrivers/intel.h>
#include <ethdrivers/virtio_pci.h>
#include <netif/etharp.h>
#include <lwip/udp.h>
#include "Environ.h"
#include "Net.h"
#include <sel4utils/thread.h>


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


static NetworkDriver _driver = {0};
lwip_iface_t lwip_driver;

static void
handle_irq(void *state, int irq_num)
{
    ethif_lwip_handle_irq(state, irq_num);
}




static struct udp_pcb udp;
static struct udp_pcb* _udp = &udp; 

static sel4utils_thread_t netThread;

static void _on_udp(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    printf("RECV from %s:%i '%s'\n",ipaddr_ntoa(addr), port, p->payload);

    udp_sendto(pcb, p, addr, port);
    pbuf_free(p);
}

static void threadStart(void *arg0, void *arg1, void *ipc_buf);


void NetInit(uint32_t iobase0)
{
    lwip_init();
    udp_init();

    LWIP_DEBUG_ENABLED(LWIP_DBG_LEVEL_ALL);
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

// add interface

    ip_addr_t addr, gw, mask;
    ipaddr_aton("192.168.0.1",   &gw);
    ipaddr_aton("10.0.2.15", &addr);
    ipaddr_aton("255.255.255.*", &mask);

    assert(lwip_driver.netif == NULL);
    lwip_driver.netif = malloc(sizeof(struct netif));
    assert(lwip_driver.netif);
    //lwip_driver.netif = netif_add_noaddr(lwip_driver.netif, _driver.driver, _driver.init_fn, ethernet_input);
    netif_add(lwip_driver.netif, &addr, &mask, &gw, _driver.driver, _driver.init_fn, ethernet_input);
    assert(lwip_driver.netif);
    netif_set_up(lwip_driver.netif);
    netif_set_default(lwip_driver.netif);

    printf("NETIF is %i\n", netif_is_up(lwip_driver.netif));
   // netif_ip4_addr(netif)
// create udp stack

    printf("Begin UDP stack init\n");
    _udp = udp_new();
    assert(_udp);

    udp_recv(_udp, _on_udp, NULL);
    err_t error_bind = udp_bind(_udp, NULL, 3000);
    assert(error_bind == ERR_OK);


// New thread
    sel4utils_thread_config_t thConf = thread_config_new(&env->simple);
    thConf = thread_config_cspace(thConf, simple_get_cnode(&env->simple), 0);

    error = sel4utils_configure_thread_config(&env->vka , &env->vspace , /*alloc*/&env->vspace , thConf , &netThread);
    assert(error == 0);

    seL4_TCB_SetPriority(netThread.tcb.cptr, seL4_CapInitThreadTCB ,  254);
    error = sel4utils_start_thread(&netThread , threadStart , irq_aep , irq , 1);
    assert(error == 0);

    printf("<----NetInit\n");

}


static void threadStart(void *arg0, void *arg1, void *ipc_buf)
{
    seL4_CPtr irq_aep = (seL4_CPtr) arg0;
    int irq = (int) arg1;

    
    seL4_MessageInfo_t msg = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_Send(irq_aep, msg);


    while(1)
    {
	    seL4_Wait(irq_aep,NULL);
        seL4_IRQHandler_Ack(irq);

         _driver.handle_irq_fn(_driver.driver, _driver.irq_num);
    }

}
