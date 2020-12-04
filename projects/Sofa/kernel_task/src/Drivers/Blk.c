
#include <stdio.h>
#include <platsupport/io.h>
#include <ethdrivers/virtio/virtio_ring.h>
#include <ethdrivers/virtio/virtio_pci.h>
#include <ethdrivers/helpers.h>
#include "Blk.h"
#include "Environ.h"
#include "DeviceTree.h"
#include "ext2.h"

#define VIRTIO_BLK_S_OK       0
#define VIRTIO_BLK_S_IOERR    1
#define VIRTIO_BLK_S_UNSUPP   2

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
    struct vring rx_ring;

    uintptr_t rx_ring_phys;


    /* R/T Descriptor Head represents the beginning of the block of
     * descriptors that are currently in use */
    //unsigned int rdh;

    /* R/T Descriptor Tail represents the next free slot to add
     * a descriptor */
    unsigned int rdt;

    /* R/T Used Head represents the index in the used ring that
     * we last observed */
    //uint16_t ruh;

    
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


static ssize_t _blkRead(IODevice* dev, size_t sector, char* buf, size_t bufSize);
static ssize_t _blkWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize);

static IODeviceOperations blkOps = 
{
    .read = _blkRead,
    .write = _blkWrite
};


VirtioDevice dev;
static IODevice _blk = IODeviceInit("virtio-blk-pci", IODevice_BlockDev, &blkOps);


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

    dma_addr_t rx_ring = dma_alloc_pin(dma_man, sizeVRing, 1, VIRTIO_PCI_VRING_ALIGN);
    if (!rx_ring.phys) {
        LOG_ERROR("Failed to allocate rx_ring");
        return -1;
    }
    memset(rx_ring.virt, 0, vring_size(dev->queueSize, VIRTIO_PCI_VRING_ALIGN));
    vring_init(&dev->rx_ring, dev->queueSize, rx_ring.virt, VIRTIO_PCI_VRING_ALIGN);
    dev->rx_ring_phys = rx_ring.phys;

    //dev->rdh = 0;
    dev->rdt = 0;
    //dev->ruh = 0;

    return 0;
}

static void blk_debug(VirtioDevice *dev)
{
    for(int i=0;i<8;i++)
    {
        printf("%i %i %c\n",i, dev->rx_ring.desc[i].len, i==dev->rdt?'*': ' ');
    }
}

static void* _blkCmd(VirtioDevice* dev, int op, size_t sector, char* buf, size_t bufSize)
{
    memset(dev->headerReq, 0, sizeof(virtio_blk_req));
    dev->headerReq->type == op;// VIRTIO_BLK_T_IN or VIRTIO_BLK_T_OUT 
    dev->headerReq->sector = sector;
    dev->headerReq->status = 12; // set to a random val


    assert(dev->hdr_phys);

    /* request a buffer */
    void *cookie = NULL; // don't care
    dma_addr_t dma_data = dma_alloc_pin(&dev->dma_man, bufSize,1, dev->dma_alignment);

    uintptr_t phys = dma_data.phys;// driver->i_cb.allocate_rx_buf(driver->cb_cookie, BUF_SIZE, &cookie);
    if (!phys) 
    {
        return NULL;
    }
    printf("-->DMA Phys %u\n", phys);
    if(op == VIRTIO_BLK_T_OUT)
    {
        printf("Copy write buffer\n");
        memcpy(dma_data.virt, buf, bufSize);
        dev->headerReq->type |= VIRTIO_BLK_T_FLUSH;
    }
    else
    {
        memset(dma_data.virt, 0, bufSize);
    }

    assert(phys % dev->dma_alignment == 0);

    uintptr_t footerPhys = dev->hdr_phys + VIRTIO_BLK_REQ_HEADER_SIZE;
    if(footerPhys % dev->dma_alignment !=0)
    {
        footerPhys += dev->dma_alignment - footerPhys % dev->dma_alignment;
    }

    assert(footerPhys % dev->dma_alignment == 0);

    unsigned int rdt_data = (dev->rdt + 1) % dev->queueSize;
    unsigned int rdt_footer = (dev->rdt + 2) % dev->queueSize;

    printf("Start CMD at %i, data %i footer %i\n", dev->rdt, rdt_data, rdt_footer);

    dev->rx_ring.desc[dev->rdt] = (struct vring_desc) {
        .addr = dev->hdr_phys,
        .len = VIRTIO_BLK_REQ_HEADER_SIZE,
        .flags = VRING_DESC_F_NEXT,
        .next = rdt_data
    };

    int dataFlag = VRING_DESC_F_NEXT;
    if(op == VIRTIO_BLK_T_IN)
    {
        dataFlag |= VRING_DESC_F_WRITE;
    }
    dev->rx_ring.desc[rdt_data] = (struct vring_desc) {
        .addr = phys,
        .len = VIRTIO_BLK_SECTOR_SIZE,
        .flags = dataFlag,
        .next = rdt_footer
    };

    dev->rx_ring.desc[rdt_footer] = (struct vring_desc) {
        .addr = footerPhys,
        .len = 1,
        .flags = VRING_DESC_F_WRITE,
        .next = 0
    };

    dev->rx_ring.avail->ring[dev->rx_ring.avail->idx % dev->queueSize] = dev->rdt;

    asm volatile("sfence" ::: "memory");
    dev->rx_ring.avail->idx++;// = dev->rdt;
    asm volatile("sfence" ::: "memory");
    assert(dev->queueID == 0);
    write_reg16(dev, VIRTIO_PCI_QUEUE_NOTIFY, dev->queueID);
    dev->rdt = (dev->rdt + 1) % dev->queueSize;
    printf("queue index is now at %i\n", dev->rx_ring.avail->idx);
    blk_debug(dev);
    return dma_data.virt;
}



static ssize_t _blkReadSector(struct _IODevice* device, size_t sector, char* buf, size_t bufSize)
{
    VirtioDevice *dev = device->impl;

    int i = dev->rx_ring.used->idx;
    void* dma_virt = _blkCmd(dev, VIRTIO_BLK_T_IN, sector, buf, bufSize);

    printf("Receiving at %i\n", i);

    virtio_blk_req *req = dev->headerReq;// .rx_ring.desc[desc1].addr;
    if(req->status != VIRTIO_BLK_S_OK)
    {
        printf("Error status not OK %i\n", req->status);
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }


    uint32_t desc1 = dev->rx_ring.used->ring[i].id;
    uint32_t desc2 = 0;
    uint32_t desc3 = 0;
    printf("desc1 at %i\n", desc1);
    printf("desc1 size %i\n", dev->rx_ring.desc[desc1].len);
    if(!(dev->rx_ring.desc[desc1].flags & VRING_DESC_F_NEXT))
    {
        printf("desc1 is missing the Next desc flag!\n");
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);
        return -1;
    }
    desc2 = dev->rx_ring.desc[desc1].next;
    printf("desc2 at %i\n", desc2);
    printf("desc2 size %i\n", dev->rx_ring.desc[desc2].len);
    printf("-->DATA Phys %u\n", dev->rx_ring.desc[desc2].addr);

    if(!(dev->rx_ring.desc[desc2].flags & VRING_DESC_F_NEXT))
    {
        printf("desc2 is missing the Next desc flag!\n");
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }
    desc3 = dev->rx_ring.desc[desc2].next;
    printf("desc3 at %i\n", desc3);
    printf("desc3 size %i\n", dev->rx_ring.desc[desc3].len);
    if(dev->rx_ring.desc[desc2].len != VIRTIO_BLK_SECTOR_SIZE)
    {
        printf("desc2' size is not VIRTIO_BLK_SECTOR_SIZE but %i\n", dev->rx_ring.desc[desc2].len);
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }

    if(dev->rx_ring.desc[desc3].len != VIRTIO_BLK_REQ_FOOTER_SIZE)
    {
        printf("desc3' size is not VIRTIO_BLK_REQ_FOOTER_SIZE but %i\n", dev->rx_ring.desc[desc3].len);
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }
    printf("Copy read buffer\n");
    memcpy(buf, dma_virt, bufSize);


    dma_unpin_free(&dev->dma_man, dma_virt, bufSize);      

    dev->rx_ring.desc[desc1].addr = 0;
    dev->rx_ring.desc[desc1].len = 0;
    dev->rx_ring.desc[desc1].next = 0;
    dev->rx_ring.desc[desc1].flags = 0;

    dev->rx_ring.desc[desc2].addr = 0;
    dev->rx_ring.desc[desc2].len = 0;
    dev->rx_ring.desc[desc2].next = 0;
    dev->rx_ring.desc[desc2].flags = 0;


    dev->rx_ring.desc[desc3].addr = 0;
    dev->rx_ring.desc[desc3].len = 0;
    dev->rx_ring.desc[desc3].next = 0;
    dev->rx_ring.desc[desc3].flags = 0;
  

    return bufSize;
}

static ssize_t _blkRead(struct _IODevice* device, size_t sector, char* buf, size_t bufSize)
{
    assert(bufSize % VIRTIO_BLK_SECTOR_SIZE == 0);
    if(bufSize <= VIRTIO_BLK_SECTOR_SIZE)
    {
        return _blkReadSector(device, sector, buf, bufSize);
    }
    const size_t numSectors = bufSize / VIRTIO_BLK_SECTOR_SIZE;
    printf("BLK.read split into %i sectors\n", numSectors);
    ssize_t tot = 0;
    for(size_t i=0;i <numSectors;i++)
    {
        ssize_t r = _blkReadSector(device, sector+i, buf + (VIRTIO_BLK_SECTOR_SIZE*i), VIRTIO_BLK_SECTOR_SIZE);
        if(r <= 0)
        {
            return r;
        }
        tot += r;
    }
    return tot;
}

static ssize_t _blkWrite(IODevice* device, size_t sector, const char* buf, size_t bufSize)
{
    VirtioDevice *dev = device->impl;

    int i = dev->rx_ring.used->idx;
    void* dma_virt = _blkCmd(dev, VIRTIO_BLK_T_OUT, sector, buf, bufSize);


    printf("receiving (writing) at %i\n", i);

    virtio_blk_req *req = dev->headerReq;// .rx_ring.desc[desc1].addr;
    if(req->status != VIRTIO_BLK_S_OK)
    {
        printf("Error status not OK %i\n", req->status);
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }

    uint32_t desc1 = dev->rx_ring.used->ring[i].id;
    uint32_t desc2 = 0;
    uint32_t desc3 = 0;
    printf("desc1 at %i\n", desc1);
    printf("desc1 size %i\n", dev->rx_ring.desc[desc1].len);
    if(!(dev->rx_ring.desc[desc1].flags & VRING_DESC_F_NEXT))
    {
        printf("desc1 is missing the Next desc flag!\n");
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);
        return -1;
    }
    desc2 = dev->rx_ring.desc[desc1].next;
    printf("desc2 at %i\n", desc2);
    printf("desc2 size %i\n", dev->rx_ring.desc[desc2].len);

    if(!(dev->rx_ring.desc[desc2].flags & VRING_DESC_F_NEXT))
    {
        printf("desc2 is missing the Next desc flag!\n");
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }
    printf("-->DATA Phys %u\n", dev->rx_ring.desc[desc2].addr);

    desc3 = dev->rx_ring.desc[desc2].next;
    printf("desc3 at %i\n", desc3);
    printf("desc3 size %i\n", dev->rx_ring.desc[desc3].len);


    if(dev->rx_ring.desc[desc2].len != VIRTIO_BLK_SECTOR_SIZE)
    {
        printf("desc2' size is not VIRTIO_BLK_SECTOR_SIZE but %i\n", dev->rx_ring.desc[desc2].len);
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }

    if(dev->rx_ring.desc[desc3].len != VIRTIO_BLK_REQ_FOOTER_SIZE)
    {
        printf("desc3' size is not VIRTIO_BLK_REQ_FOOTER_SIZE but %i\n", dev->rx_ring.desc[desc3].len);
        dma_unpin_free(&dev->dma_man, dma_virt, bufSize);        
        return -1;
    }


    dma_unpin_free(&dev->dma_man, dma_virt, bufSize);
}


int BlkInit(uint32_t iobase)
{
    printf("==> Init Blk virtio storage\n");
    KernelTaskContext* env = getKernelTaskContext();

    _blk.impl = &dev;
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



    add_status(&dev, VIRTIO_CONFIG_S_DRIVER_FEATURES_OK);
    asm volatile("mfence" ::: "memory");

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

    assert(dev.rx_ring_phys % 4096 == 0);

    write_reg16(&dev, VIRTIO_PCI_QUEUE_SEL, 0);
    write_reg32(&dev, VIRTIO_PCI_QUEUE_PFN, ((uintptr_t)dev.rx_ring_phys) / 4096);

    add_status(&dev, VIRTIO_CONFIG_S_DRIVER_OK);

    
    printf("Dev status %u\n", get_status(&dev));

    DeviceTreeAddDevice(&_blk);

    if(!(get_status(&dev) & VIRTIO_CONFIG_S_DRIVER_OK))
	{
		printf("DEV status not driver ok\n");
        assert(0);
    }
	else
	{
		printf("Dev good to go\n");
	}

    return 0;
}