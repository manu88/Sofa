
#include <stdio.h>
#include <platsupport/io.h>
#include <ethdrivers/virtio/virtio_ring.h>
#include <ethdrivers/virtio/virtio_pci.h>
#include <ethdrivers/helpers.h>
#include "Blk.h"
#include "Environ.h"


typedef struct
{
    uint32_t iobase;
    ps_io_port_ops_t ioops;
    
    uint8_t queueID;
    unsigned int rx_remain;
    struct vring rx_ring;
    uintptr_t rx_ring_phys;
    void **rx_cookies;

    unsigned int *tx_lengths;

    /* R/T Descriptor Head represents the beginning of the block of
     * descriptors that are currently in use */
    unsigned int rdh;

    /* R/T Descriptor Tail represents the next free slot to add
     * a descriptor */
    unsigned int rdt;

    /* R/T Used Head represents the index in the used ring that
     * we last observed */
    uint16_t ruh;

    
    size_t queueSize;


    ps_dma_man_t dma_man;
    int num_free_bufs;
    dma_addr_t **bufs;
    int dma_alignment;
} VirtioDevice;




typedef struct 
{
#define VIRTIO_BLK_T_IN       0
#define VIRTIO_BLK_T_OUT      1
#define VIRTIO_BLK_T_SCSI     2
#define VIRTIO_BLK_T_FLUSH    4
	uint32_t type;
	uint32_t reserved;
	uint64_t sector;
	uint8_t status;
} __attribute__((packed)) virtio_blk_req;

/* An 8-bit device status register.  */
#define VIRTIO_PCI_STATUS		18

#define VIRTIO_CONFIG_S_ACKNOWLEDGE	1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER		2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

#define VIRTIO_PCI_HOST_FEATURES	0


#define BLK_NUM_PREALLOCATED_BUFFERS 512
#define BLK_PREALLOCATED_BUF_SIZE 2048



static void write_reg8(VirtioDevice *dev, uint16_t port, uint8_t val) 
{
    ps_io_port_out(&dev->ioops, dev->iobase + port, 1, val);
}

static uint8_t read_reg8(VirtioDevice *dev, uint16_t port) 
{
    uint32_t val;
    ps_io_port_in(&dev->ioops, dev->iobase + port, 1, &val);
    return (uint8_t)val;
}

static void write_reg16(VirtioDevice *dev, uint16_t port, uint16_t val) {
    ps_io_port_out(&dev->ioops, dev->iobase + port, 2, val);
}

static uint16_t read_reg16(VirtioDevice *dev, uint16_t port) {
    uint32_t val;
    ps_io_port_in(&dev->ioops, dev->iobase + port, 2, &val);
    return (uint16_t)val;
}

static uint32_t read_reg32(VirtioDevice *dev, uint16_t port) {
    uint32_t val;
    ps_io_port_in(&dev->ioops, dev->iobase + port, 4, &val);
    return val;
}

static void set_status(VirtioDevice *dev, uint8_t status) 
{
    write_reg8(dev, VIRTIO_PCI_STATUS, status);
}

static uint8_t get_status(VirtioDevice *dev) 
{
    return read_reg8(dev, VIRTIO_PCI_STATUS);
}

static void add_status(VirtioDevice *dev, uint8_t status) {
    write_reg8(dev, VIRTIO_PCI_STATUS, get_status(dev) | status);
}

static uint32_t get_features(VirtioDevice *dev) {
    return read_reg32(dev, VIRTIO_PCI_HOST_FEATURES);
}


static int initialize_desc_ring(VirtioDevice *dev, ps_dma_man_t *dma_man) {

    dma_addr_t rx_ring = dma_alloc_pin(dma_man, vring_size(dev->queueSize, VIRTIO_PCI_VRING_ALIGN), 1, VIRTIO_PCI_VRING_ALIGN);
    if (!rx_ring.phys) {
        LOG_ERROR("Failed to allocate rx_ring");
        return -1;
    }
    memset(rx_ring.virt, 0, vring_size(dev->queueSize, VIRTIO_PCI_VRING_ALIGN));
    vring_init(&dev->rx_ring, dev->queueSize, rx_ring.virt, VIRTIO_PCI_VRING_ALIGN);
    dev->rx_ring_phys = rx_ring.phys;
    dev->rx_cookies = malloc(sizeof(void*) * dev->queueSize);
    dev->tx_lengths = malloc(sizeof(unsigned int) * dev->queueSize);
    if (!dev->rx_cookies || !dev->tx_lengths) 
    {
        LOG_ERROR("Failed to malloc");
        //free_desc_ring(dev, dma_man);
        return -1;
    }
    /* Remaining needs to be 2 less than size as we cannot actually enqueue size many descriptors,
     * since then the head and tail pointers would be equal, indicating empty. */
    dev->rx_remain = dev->queueSize - 2;

    dev->rdh = 0;
    dev->rdt = 0;
    dev->ruh = 0;

    return 0;
}

static int initialize_free_bufs(VirtioDevice *dev)
{
    dma_addr_t *dma_bufs = NULL;
    dma_bufs = malloc(sizeof(dma_addr_t) * BLK_NUM_PREALLOCATED_BUFFERS);
    if (!dma_bufs) 
    {
        printf("Unable to malloc blk dma_bufs\n");
        goto error;
    }
    memset(dma_bufs, 0, sizeof(dma_addr_t) * BLK_NUM_PREALLOCATED_BUFFERS);
    dev->bufs = malloc(sizeof(dma_addr_t *) * BLK_NUM_PREALLOCATED_BUFFERS);
    if (!dev->bufs) 
    {
        printf("Unable to malloc blk dev->bufs\n");
        goto error;
    }
    for (int i = 0; i < BLK_NUM_PREALLOCATED_BUFFERS; i++) {
        dma_bufs[i] = dma_alloc_pin(&dev->dma_man, BLK_PREALLOCATED_BUF_SIZE, 1,
                                    dev->dma_alignment);
        if (!dma_bufs[i].phys) 
        {
            printf("No phys addr for dma buf %i\n", i);
            goto error;
        }
        ps_dma_cache_clean_invalidate(&dev->dma_man, dma_bufs[i].virt, BLK_PREALLOCATED_BUF_SIZE);
        dev->bufs[i] = &dma_bufs[i];
    }
    dev->num_free_bufs = BLK_NUM_PREALLOCATED_BUFFERS;
    return 0;  
error:
    if (dev->bufs) 
    {
        free(dev->bufs);
    }
    if (dma_bufs) {
        for (int i = 0; i < BLK_NUM_PREALLOCATED_BUFFERS; i++) {
            if (dma_bufs[i].virt) {
                dma_unpin_free(&dev->dma_man, dma_bufs[i].virt, BLK_PREALLOCATED_BUF_SIZE);
            }
        }
        free(dma_bufs);
    }
    dev->bufs = NULL;
    return -1;
}

static void complete_tx(VirtioDevice *dev) 
{
    while (dev->ruh != dev->rx_ring.used->idx) 
    {
        uint16_t ring = dev->ruh % dev->queueSize;
        unsigned int UNUSED desc = dev->rx_ring.used->ring[ring].id;
        assert(desc == dev->rdh);
        void *cookie = dev->rx_cookies[dev->rdh];
        /* add 1 to the length we stored to account for the extra descriptor
         * we used for the virtio header */
        unsigned int used = dev->tx_lengths[dev->rdh] + 1;
        dev->rx_remain += used;
        dev->rdh = (dev->rdh + used) % dev->queueSize;
        dev->ruh++;
        /* give the buffer back */
        //driver->i_cb.tx_complete(driver->cb_cookie, cookie);
    }
}


static int raw_tx(VirtioDevice *dev, unsigned int num, uintptr_t *phys, unsigned int *len, void *cookie) 
{
    /* we need to num + 1 free descriptors. The + 1 is for the virtio header */
    if (dev->rx_remain < num + 1) {
        complete_tx(dev);
        if (dev->rx_remain < num + 1) 
        {
            return -1;
        }
    }
    /* install the header */
    /*
    dev->rx_ring.desc[dev->rdt] = (struct vring_desc) 
    {
        .addr = dev->virtio_net_hdr_phys,
        .len = sizeof(struct virtio_net_hdr),
        .flags = VRING_DESC_F_NEXT,
        .next = (dev->tdt + 1) % dev->tx_size
    };
    */
    /* now all the buffers */
    unsigned int i;
    for (i = 0; i < num; i++) {
        unsigned int desc = (dev->rdt + i + 1) % dev->queueSize;
        unsigned int next_desc = (desc + 1) % dev->queueSize;
        dev->rx_ring.desc[desc] = (struct vring_desc) 
        {
            .addr = phys[i],
            .len = len[i],
            .flags = (i + 1 == num ? 0 : VRING_DESC_F_NEXT),
            .next = next_desc
        };
    }
    dev->rx_ring.avail->ring[dev->rx_ring.avail->idx% dev->queueSize] = dev->rdt;
    dev->rx_cookies[dev->rdt] = cookie;
    dev->tx_lengths[dev->rdt] = num;
    /* ensure update to descriptors visible before updating the index */
    asm volatile("mfence" ::: "memory");
    dev->rdt = (dev->rdt + num + 1) % dev->queueSize;
    dev->rx_remain -= (num + 1);
    dev->rx_ring.avail->idx++;
    /* ensure index update visible before notifying */
    asm volatile("mfence" ::: "memory");
    write_reg16(dev, VIRTIO_PCI_QUEUE_NOTIFY, dev->queueID);
    return 0;
}


static void send_cmd(VirtioDevice* dev)
{
    printf("Send cmd test\n");


    dev->num_free_bufs--;
    printf("Add dma buffer %i\n", dev->num_free_bufs);
    dma_addr_t *orig_buf = dev->bufs[dev->num_free_bufs];
    dma_addr_t buf = *orig_buf;

    virtio_blk_req* req = (virtio_blk_req*) buf.virt;
    memset(req, 0, sizeof(virtio_blk_req));
    req->type == VIRTIO_BLK_T_IN; 
    req->sector = 0;

    size_t dataSize = sizeof(virtio_blk_req);
    ps_dma_cache_clean(&dev->dma_man, buf.virt, dataSize);



    printf("End cmd test\n");
}

void BlkInit(uint32_t iobase)
{
    printf("==> Init Blk virtio storage\n");
    KernelTaskContext* env = getKernelTaskContext();

    VirtioDevice dev;
    dev.iobase = iobase;
    dev.ioops = env->ops.io_port_ops;

    dev.dma_man = env->ops.dma_manager;
    dev.dma_alignment = 16;
    // reset
    set_status(&dev, 0);
    uint8_t stat =  get_status(&dev);
    /* acknowledge to the host that we found it */
    add_status(&dev, VIRTIO_CONFIG_S_ACKNOWLEDGE);
    add_status(&dev, VIRTIO_CONFIG_S_DRIVER);

    uint32_t feats = get_features(&dev);

    uint32_t totSectorCount = read_reg32(&dev, 0x14);
    uint8_t maxSegSize      = read_reg8(&dev, 0x1C);
    uint8_t maxSegCount     = read_reg8(&dev, 0x20);
    uint8_t cylinderCount   = read_reg8(&dev, 0x24);
    uint8_t headCount       = read_reg8(&dev, 0x26);
    uint8_t sectorCount     = read_reg8(&dev, 0x27);
    uint8_t blockLen        = read_reg8(&dev, 0x28);


    printf("totSectorCount %u\n", totSectorCount);
    printf("maxSegSize %u\n", maxSegSize);
    printf("maxSegCount %u\n", maxSegCount);
    printf("cylinderCount %u\n", cylinderCount);
    printf("headCount %u\n", headCount);
    printf("sectorCount %u\n", sectorCount);
    printf("blockLen %u\n", blockLen);

    uint8_t numQueues = 0;
    for(int index = 0;index<16;index++)
    {
        write_reg16(&dev, VIRTIO_PCI_QUEUE_SEL, index);
        uint16_t queueSize = read_reg16(&dev, VIRTIO_PCI_QUEUE_NUM);
        if(queueSize == 0)
        {
            continue;
        }
        dev.queueSize = queueSize;
        dev.queueID = index;
        numQueues++;
        printf("Queue %i size %i\n",index, queueSize);
    }
    printf("Virtio blk has %i available queues\n", numQueues);
    
        
    int err = initialize_desc_ring(&dev, &env->ops.dma_manager);
    assert(err == 0);

    printf("Init free buffers\n");
    err = initialize_free_bufs(&dev);
    assert(err == 0);
    printf("Init free buffers OK\n");
    add_status(&dev, VIRTIO_CONFIG_S_DRIVER_OK);


    send_cmd(&dev);
    int irq_num = 11;
    /**/
    cspacepath_t irq_path = { 0 };
    seL4_CPtr irq;

    int error = vka_cspace_alloc(&env->vka, &irq);
    assert(error == 0);
    vka_cspace_make_path(&env->vka, irq, &irq_path);

    error = simple_get_IRQ_handler(&env->simple, irq_num, irq_path);
    assert(error == 0);

    vka_object_t irq_aep_obj = { 0 };

    error = vka_alloc_notification(&env->vka, &irq_aep_obj);
    assert(error == 0);

    seL4_CPtr irq_aep = irq_aep_obj.cptr;
    error = seL4_IRQHandler_SetNotification(irq_path.capPtr, irq_aep);
    assert(error == 0);

    printf("<== End Init Blk virtio storage\n");

    while(1)
    {
	    seL4_Wait(irq_aep,NULL);
        seL4_IRQHandler_Ack(irq);
        printf("blk irq\n");
         //_driver.handle_irq_fn(_driver.driver, _driver.irq_num);
    }

}