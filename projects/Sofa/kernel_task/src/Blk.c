
#include <stdio.h>
#include <platsupport/io.h>
#include <ethdrivers/virtio/virtio_ring.h>
#include <ethdrivers/virtio/virtio_pci.h>
#include <ethdrivers/helpers.h>
#include "Blk.h"
#include "Environ.h"


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
//    int num_free_bufs;
//    dma_addr_t **bufs;
    int dma_alignment;


    /* preallocated header. Since we do not actually use any features
     * in the header we put the same one before every send/receive packet */
    uintptr_t hdr_phys;
    virtio_blk_req* headerReq;
} VirtioDevice;





/* An 8-bit device status register.  */
#define VIRTIO_PCI_STATUS		18

#define VIRTIO_CONFIG_S_ACKNOWLEDGE	1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER		2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

#define VIRTIO_PCI_HOST_FEATURES	0

#define VIRTIO_CONFIG_S_DRIVER_FEATURES_OK 8


#define BLK_NUM_PREALLOCATED_BUFFERS 512
#define BLK_PREALLOCATED_BUF_SIZE 2048

#define VIRTIO_BLK_SECTOR_SIZE 512

#define VIRTIO_BLK_REQ_HEADER_SIZE 16
#define VIRTIO_BLK_REQ_FOOTER_SIZE 1


/* An alignment of 128 bytes is required for most structures by the hardware, except for actual packets */
// This is X86_64 specific!
#define DMA_ALIGN 128


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

static void write_reg32(VirtioDevice *dev, uint16_t port, uint32_t val) {
    ps_io_port_out(&dev->ioops, dev->iobase + port, 4, val);
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

static void set_features(VirtioDevice *dev, uint32_t features) {
    write_reg32(dev, VIRTIO_PCI_GUEST_FEATURES, features);
}


static int initialize_desc_ring(VirtioDevice *dev, ps_dma_man_t *dma_man) {

    unsigned sizeVRing = vring_size(dev->queueSize, VIRTIO_PCI_VRING_ALIGN);
    printf("vring_size=%u\n", sizeVRing);
    dma_addr_t rx_ring = dma_alloc_pin(dma_man, sizeVRing, 1, VIRTIO_PCI_VRING_ALIGN);
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

#if 0

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
#endif
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
    printf("add header at %i\n",dev->rdt);
    dev->rx_ring.desc[dev->rdt] = (struct vring_desc) 
    {
        .addr = dev->hdr_phys,
        .len = VIRTIO_BLK_REQ_HEADER_SIZE,
        .flags = VRING_DESC_F_NEXT,
        .next = (dev->rdt + 1) % dev->queueSize
    };


    /* now all the buffers */
    unsigned int i;
    for (i = 0; i < num; i++) 
    {
        unsigned int desc = (dev->rdt + i + 1) % dev->queueSize;
        printf("raw_tx add buf %i/%i at %i\n", i, num,desc);
        unsigned int next_desc = (desc + 1) % dev->queueSize;

        uint8_t isLast = (i+1) == num;

        uint16_t flags = VRING_DESC_F_WRITE | VRING_DESC_F_NEXT;
        if(isLast)
        {
            flags = VRING_DESC_F_WRITE;
        }

        printf("raw_tx add buf %i/%i at %i : len %zi flags %i\n", i, num,desc, len[i], flags);

        dev->rx_ring.desc[desc] = (struct vring_desc) 
        {
            .addr = phys[i],
            .len = len[i],
            .flags = flags,//(i + 1 == num ? VRING_DESC_F_WRITE : VRING_DESC_F_NEXT),
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
    printf("NOTIFY queue %i, remains %i\n", dev->queueID, dev->rx_remain);
    write_reg16(dev, VIRTIO_PCI_QUEUE_NOTIFY, dev->queueID);
    return 0;
}


static void send_cmd2(VirtioDevice* dev, int sector)
{
        memset(dev->headerReq, 0, sizeof(virtio_blk_req));
        dev->headerReq->type == VIRTIO_BLK_T_IN; 
        dev->headerReq->sector = sector;

        assert(dev->hdr_phys);

        /* request a buffer */
        void *cookie;
        dma_addr_t dma_data = dma_alloc_pin(&dev->dma_man, 1024,1, dev->dma_alignment);

        uintptr_t phys = dma_data.phys;// driver->i_cb.allocate_rx_buf(driver->cb_cookie, BUF_SIZE, &cookie);
        if (!phys) 
        {
            assert(0);
            return;
        }
        unsigned int rdt_data = (dev->rdt + 1) % dev->queueSize;
        unsigned int rdt_footer = (dev->rdt + 2) % dev->queueSize;

        printf("%i %i %i\n", dev->rdt, rdt_data, rdt_footer);

        dev->rx_ring.desc[dev->rdt] = (struct vring_desc) {
            .addr = dev->hdr_phys,
            .len = VIRTIO_BLK_REQ_HEADER_SIZE,
            .flags = VRING_DESC_F_NEXT,
            .next = rdt_data
        };

        dev->rx_cookies[rdt_data] = cookie;
        dev->rx_ring.desc[rdt_data] = (struct vring_desc) {
            .addr = phys,
            .len = 512,
            .flags = VRING_DESC_F_NEXT |VRING_DESC_F_WRITE,
            .next = rdt_footer
        };

        dev->rx_cookies[rdt_footer] = cookie;
        dev->rx_ring.desc[rdt_footer] = (struct vring_desc) {
            .addr = phys + VIRTIO_BLK_REQ_HEADER_SIZE,
            .len = 1,
            .flags = VRING_DESC_F_WRITE,
            .next = 0
        };


        dev->rx_ring.avail->ring[dev->rx_ring.avail->idx % dev->queueSize] = dev->rdt;
        asm volatile("sfence" ::: "memory");
        dev->rx_ring.avail->idx+3;
        asm volatile("sfence" ::: "memory");
        write_reg16(dev, VIRTIO_PCI_QUEUE_NOTIFY, dev->queueID);
        dev->rdt = (dev->rdt + 3) % dev->queueSize;
        dev->rx_remain-=3;
}

static void send_cmd(VirtioDevice* dev)
{
    printf("Send cmd test\n");


    dev->headerReq->type == VIRTIO_BLK_T_IN; 
    dev->headerReq->sector = 1;

    dma_addr_t dma_data = dma_alloc_pin(&dev->dma_man, 512,1, dev->dma_alignment);

    uintptr_t phys[] = {
        dma_data.phys,// ps_dma_pin(&dev->dma_man, p, 512),
        dev->hdr_phys + 16//ps_dma_pin(&dev->dma_man, p + 512, 1)
    };

    unsigned int len[] = {
        512,
        1
    };

    assert(phys[0]);
    assert(phys[1]);
    int err = raw_tx(dev,  2, phys, len, NULL);

    printf("End cmd test err=%i\n", err);
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
    set_features(&dev, feats);

    add_status(&dev, VIRTIO_CONFIG_S_DRIVER_FEATURES_OK);

#define VIRTIO_F_VERSION_1              1 << 32
#define VIRTIO_F_RING_EVENT_IDX         1<<29

    printf("BEFORE Features %X\n", feats);
    if(!(feats & VIRTIO_F_VERSION_1))
    {
        printf("VIRTIO_F_VERSION_1 NOT PRESENT\n");
    }

    if(!(feats & VIRTIO_F_RING_EVENT_IDX))
    {
        printf("VIRTIO_F_RING_EVENT_IDX NOT PRESENT\n");
    }
    else
    {
        printf("VIRTIO_F_RING_EVENT_IDX set\n");
    }
    printf("feats 1 %X\n", feats);
    //feats &= ~(VIRTIO_F_RING_EVENT_IDX);
    printf("feats 2 %X\n", feats);
    set_features(&dev, feats);

    feats = get_features(&dev);
    printf("AFTER Features %X\n", feats);


    add_status(&dev, VIRTIO_CONFIG_S_DRIVER_FEATURES_OK);
    asm volatile("mfence" ::: "memory");
    if(!(get_status(&dev) & VIRTIO_CONFIG_S_DRIVER_FEATURES_OK))
	{
		printf("HOST REFUSED OUR FEATURES\n");
        assert(0);
    }
	else
	{
		printf("HOST is ok with our features\n");
	}


    uint32_t totSectorCount = read_reg32(&dev, 0x14);
    uint8_t maxSegSize      =  read_reg8(&dev, 0x1C);
    uint8_t maxSegCount     =  read_reg8(&dev, 0x20);
    uint8_t cylinderCount   =  read_reg8(&dev, 0x24);
    uint8_t headCount       =  read_reg8(&dev, 0x26);
    uint8_t sectorCount     =  read_reg8(&dev, 0x27);
    uint8_t blockLen        =  read_reg8(&dev, 0x28);


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

#if 0
    printf("Init free buffers\n");
    err = initialize_free_bufs(&dev);
    assert(err == 0);
    printf("Init free buffers OK\n");
#endif
    printf("%zi %zi\n", sizeof(virtio_blk_req), VIRTIO_BLK_REQ_HEADER_SIZE);
    dma_addr_t packet = dma_alloc_pin(&env->ops.dma_manager, sizeof(virtio_blk_req), 1, DMA_ALIGN);
    if (!packet.virt) 
    {
        printf("ERROR : unable to alloc DMA for Virtio BLK request\n");
        assert(0);
    }
    memset(packet.virt, 0, sizeof(virtio_blk_req));
    dev.hdr_phys = packet.phys;
    dev.headerReq = packet.virt;
    assert(dev.hdr_phys);
    assert(dev.headerReq);

    assert(dev.rx_ring_phys);
    /* write the virtqueue locations */
    write_reg16(&dev, VIRTIO_PCI_QUEUE_SEL, 0);
    write_reg32(&dev, VIRTIO_PCI_QUEUE_PFN, ((uintptr_t)dev.rx_ring_phys) / 4096);

    add_status(&dev, VIRTIO_CONFIG_S_DRIVER_OK);

    
    printf("Dev status %u\n", get_status(&dev));



    if(!(get_status(&dev) & VIRTIO_CONFIG_S_DRIVER_OK))
	{
		printf("DEV status not driver ok\n");
        assert(0);
    }
	else
	{
		printf("Dev good to go\n");
	}

    for (int i=0;i<1;i++)
    {
        send_cmd2(&dev, i);
    }
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


    printf("BEFORE SEND remain %i\n", dev.rx_remain);

    //seL4_MessageInfo_t msg = seL4_MessageInfo_new(0, 0, 0, 1);
    //seL4_Send(irq_aep, msg);

    while(1)
    {
	    seL4_Wait(irq_aep,NULL);
        seL4_IRQHandler_Ack(irq);
        uint8_t isr = read_reg8(&dev, VIRTIO_PCI_ISR);
        printf("blk irq ISR %u remain %i\n", isr, dev.rx_remain);

        
         //_driver.handle_irq_fn(_driver.driver, _driver.irq_num);
    }

}