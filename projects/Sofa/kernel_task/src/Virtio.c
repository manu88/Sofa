/*
From https://github.com/brenns10/sos/blob/13a2d89cb8edbb45535279d2a4f07ed74c53ec91/kernel/virtio.c
*/
/**
 * Implements virtio device drivers, particularly mmio ones.
 *
 * Reference:
 *
 * http://docs.oasis-open.org/virtio/virtio/v1.0/cs04/virtio-v1.0-cs04.html
 */
#include <string.h>
#include <stdio.h>
#include <utils/page.h>
#include <ethdrivers/helpers.h>
#include "Virtio.h"
#include "Environ.h"


#define VIRTIO_MAGIC 0x74726976
#define VIRTIO_VERSION 0x2
#define VIRTIO_DEV_BLK 0x2
#define wrap(x, len) ((x) & ~(len))


#define ALIGN_MEM(x, a)           __ALIGN_MASK(x, (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

//uint8_t buffer[512];

struct virtio_cap indp_caps[] = {
	{"VIRTIO_F_RING_INDIRECT_DESC", 1<<28, false,
		"Negotiating this feature indicates that the driver can use"
		" descriptors with the VIRTQ_DESC_F_INDIRECT flag set, as"
		" described in 2.4.5.3 Indirect Descriptors."},
	{"VIRTIO_F_RING_EVENT_IDX", 1<<29, false,
		"This feature enables the used_event and the avail_event fields"
		" as described in 2.4.7 and 2.4.8."},
	/*{"VIRTIO_F_VERSION_1", 1<<32, false,
		"This indicates compliance with this specification, giving a"
		" simple way to detect legacy devices or drivers."},*/
};

struct virtio_cap blk_caps[] = {
	{"VIRTIO_BLK_F_SIZE_MAX", 1<<1, false,
		"Maximum size of any single segment is in size_max."},
	{"VIRTIO_BLK_F_SEG_MAX", 1<<2, false,
		"Maximum number of segments in a request is in seg_max."},
	{"VIRTIO_BLK_F_GEOMETRY", 1<<4, false,
		"Disk-style geometry specified in geometry."},
	{"VIRTIO_BLK_F_RO", 1<<5, false,
		"Device is read-only."},
	{"VIRTIO_BLK_F_BLK_SIZE", 1<<6, false,
		"Block size of disk is in blk_size."},
	{"VIRTIO_BLK_F_FLUSH", 1<<9, false,
		"Cache flush command support."},
	{"VIRTIO_BLK_F_TOPOLOGY", 1<<10, false,
		"Device exports information on optimal I/O alignment."},
	{"VIRTIO_BLK_F_CONFIG_WCE", 1<<11, false,
		"Device can toggle its cache between writeback and "
		"writethrough modes."},
};


struct virtio_blk 
{
	Device _dev;
	struct virtqueue *virtq;
	uint32_t intid;
} blkdev;


struct virtqueue *virtq_create(uint32_t len)
{
	int i;
	uintptr_t page_phys;
	uintptr_t page_virt;
	struct virtqueue *virtq;

/* compute offsets */
	uintptr_t off_desc = ALIGN_MEM(sizeof(struct virtqueue), 16);
	uintptr_t off_avail =
	        ALIGN_MEM(off_desc + len * sizeof(struct virtqueue_desc), 2);
	uintptr_t off_used_event = (off_avail + sizeof(struct virtqueue_avail) +
	                           len * sizeof(uint16_t));
	uintptr_t off_used = ALIGN_MEM(off_used_event + sizeof(uint16_t), 4);
	uintptr_t off_avail_event = (off_used + sizeof(struct virtqueue_used) +
	                            len * sizeof(struct virtqueue_used_elem));
	uintptr_t off_desc_virt =
	        ALIGN_MEM(off_avail_event + sizeof(uint16_t), sizeof(void *));
	uintptr_t memsize = off_desc_virt + len * sizeof(void *);

/*
	if (memsize > PAGE_SIZE_4K) {
		printf("virtq_create: error, too big for a page %i\n", memsize);
		return NULL;
	}
*/
	KernelTaskContext* env = getKernelTaskContext();

	dma_addr_t alloc_dma = dma_alloc_pin(&env->ops.dma_manager, memsize, 1, VIRTIO_PCI_VRING_ALIGN);
	
	page_phys =  alloc_dma.phys;// alloc_pages(phys_allocator, PAGE_SIZE, 0);
	page_virt = (uintptr_t) alloc_dma.virt;// alloc_pages(kern_virt_allocator, PAGE_SIZE, 0);
//	kmem_map_pages(page_virt, page_phys, PAGE_SIZE, PRW_UNA | EXECUTE_NEVER);

	virtq = (struct virtqueue *)page_virt;
	virtq->phys = page_phys;
	virtq->len = len;

	virtq->desc = (struct virtqueue_desc *)(page_virt + off_desc);
	virtq->avail = (struct virtqueue_avail *) (page_virt + off_avail);
	virtq->used_event = (uint16_t *) (page_virt + off_used_event);
	virtq->used = (struct virtqueue_used *) (page_virt + off_used);
	virtq->avail_event = (uint16_t *) (page_virt + off_avail_event);
	virtq->desc_virt = (void **) (page_virt + off_desc_virt);

	virtq->avail->idx = 0;
	virtq->used->idx = 0;
	virtq->seen_used = virtq->used->idx;
	virtq->free_desc = 0;

	for (i = 0; i < len; i++) {
		virtq->desc[i].next = i + 1;
	}

	return virtq;
}

uint32_t virtq_alloc_desc(struct virtqueue *virtq, void *addr, size_t size)
{
	KernelTaskContext* env = getKernelTaskContext();
	assert(virtq);
	uint32_t desc = virtq->free_desc;
	uint32_t next = virtq->desc[desc].next;
	if (next == virtq->len)
		puts("ERROR: ran out of virtqueue descriptors\n");
	virtq->free_desc = next;


	virtq->desc[desc].addr = ps_dma_pin(&env->ops.dma_manager, addr, size);//  kmem_lookup_phys(addr);
	assert(virtq->desc[desc].addr);
	virtq->desc_virt[desc] = addr;
	return desc;
}

void virtq_free_desc(struct virtqueue *virtq, uint32_t desc)
{
	virtq->desc[desc].next = virtq->free_desc;
	virtq->free_desc = desc;
	virtq->desc_virt[desc] = NULL;
}

void virtq_add_to_device(struct virtqueue *virtq, uint32_t queue_sel)
{
	printf("START virtq_add_to_device\n");
	WriteReg16(&blkdev._dev, VIRTIO_PCI_QUEUE_SEL, queue_sel);
	printf("virtq_add_to_device %X\n", virtq->phys);
    WriteReg32(&blkdev._dev, VIRTIO_PCI_QUEUE_PFN, (uintptr_t)virtq->phys / 4096);
/*
	WRITE32(regs->QueueSel, queue_sel);
	mb();
	WRITE32(regs->QueueNum, virtq->len);
	WRITE32(regs->QueueDescLow, virtq->phys + ((void*)virtq->desc - (void*)virtq));
	WRITE32(regs->QueueDescHigh, 0);
	WRITE32(regs->QueueAvailLow, virtq->phys + ((void*)virtq->avail - (void*)virtq));
	WRITE32(regs->QueueAvailHigh, 0);
	WRITE32(regs->QueueUsedLow, virtq->phys + ((void*)virtq->used - (void*)virtq));
	WRITE32(regs->QueueUsedHigh, 0);
	mb();
	WRITE32(regs->QueueReady, 1);
*/
}

static int virtio_check_capabilities(uint32_t *device, uint32_t *request, struct virtio_cap *caps, uint32_t n)
{
	uint32_t i;
	for (i = 0; i < n; i++) {
		if (*device & caps[i].bit) 
		{
			if (caps[i].support) 
			{
				*request |= caps[i].bit;
			} else {
				printf("virtio supports unsupported option %s (%s)\n",
						caps[i].name, caps[i].help);
			}
		}
		*device &= ~caps[i].bit;
	}
}

#define HI32(u64) ((uint32_t)((0xFFFFFFFF00000000ULL & (u64)) >> 32))
#define LO32(u64) ((uint32_t)(0x00000000FFFFFFFFULL & (u64)))

static void virtio_blk_handle_used(struct virtio_blk *dev, uint32_t usedidx)
{
	printf("virtio_blk_handle_used for index %i\n", usedidx);
	struct virtqueue *virtq = dev->virtq;
	uint32_t desc1, desc2, desc3;
	struct virtio_blk_req *req;
	uint8_t *data;

	desc1 = virtq->used->ring[usedidx].id;
	if (!(virtq->desc[desc1].flags & VIRTQ_DESC_F_NEXT))
		goto bad_desc;
	desc2 = virtq->desc[desc1].next;
	if (!(virtq->desc[desc2].flags & VIRTQ_DESC_F_NEXT))
		goto bad_desc;
	desc3 = virtq->desc[desc2].next;
	if (virtq->desc[desc1].len != VIRTIO_BLK_REQ_HEADER_SIZE
			|| virtq->desc[desc2].len != VIRTIO_BLK_SECTOR_SIZE
			|| virtq->desc[desc3].len != VIRTIO_BLK_REQ_FOOTER_SIZE)
		goto bad_desc;

	req = virtq->desc_virt[desc1];
	data = virtq->desc_virt[desc2];
	if (req->status != VIRTIO_BLK_S_OK)
		goto bad_status;

	if (req->type == VIRTIO_BLK_T_IN) {
		printf("virtio-blk: result: \"%s\"\n", data);
	}

	virtq_free_desc(virtq, desc1);
	virtq_free_desc(virtq, desc2);
	virtq_free_desc(virtq, desc3);

	return;
bad_desc:
	puts("virtio-blk received malformed descriptors\n");
	return;

bad_status:
	puts("virtio-blk: error in command response\n");
	return;
}

static void virtio_blk_isr(uint32_t intid)
{
	/* TODO: support multiple block devices by examining intid */
	struct virtio_blk *dev = &blkdev;
	int i;
	int len = dev->virtq->len;

	//WRITE32(dev->regs->InterruptACK, READ32(dev->regs->InterruptStatus));
	printf("seen_used %i\n", dev->virtq->seen_used);
	for (i = dev->virtq->seen_used; i != dev->virtq->used->idx; i = wrap(i + 1, len)) 
	{
		virtio_blk_handle_used(dev, i);
	}
	dev->virtq->seen_used = dev->virtq->used->idx;

	//gic_end_interrupt(intid);
}

#define nelem(x) (sizeof(x) / sizeof(x[0]))

static void set_status(Device *dev, uint8_t status) 
{
    WriteReg8(dev, VIRTIO_PCI_STATUS, status);
}

static uint8_t get_status(Device *dev) 
{
    return ReadReg8(dev, VIRTIO_PCI_STATUS);
}

static void add_status(Device *dev, uint8_t status)
{
    WriteReg8(dev, VIRTIO_PCI_STATUS, get_status(dev) | status);
}

static void set_features(Device *dev, uint32_t features) {
    WriteReg32(dev, VIRTIO_PCI_GUEST_FEATURES, features);
}

static int virtio_blk_cmd(struct virtio_blk *blk, uint32_t type, uint32_t sector, uint8_t *data);

int virtio_blk_init(uint32_t iobase)
{
	printf("--> INIT Virtio BLK\n");
	KernelTaskContext* env = getKernelTaskContext();
	blkdev._dev.ioops = env->ops.io_port_ops;
	blkdev._dev.iobase = iobase;
	struct virtqueue *virtq;
	uint32_t request_features = 0;
	uint32_t DeviceFeatures;
	uint32_t i;

	set_status(&blkdev._dev, 0);
	printf("device Status is %u\n", get_status(&blkdev._dev));

	add_status(&blkdev._dev, VIRTIO_STATUS_ACKNOWLEDGE);
    add_status(&blkdev._dev, VIRTIO_STATUS_DRIVER);


	uint32_t deviceFeatures = ReadReg32(&blkdev._dev, VIRTIO_PCI_HOST_FEATURES);
	printf("Device feats %X\n", deviceFeatures);
	virtio_check_capabilities(&DeviceFeatures, &request_features, blk_caps, nelem(blk_caps));
	virtio_check_capabilities(&DeviceFeatures, &request_features, indp_caps, nelem(indp_caps));

	if (DeviceFeatures) {
		printf("virtio supports undocumented options 0x%x!\n", DeviceFeatures);
	}

	set_features(&blkdev._dev, request_features);

	add_status(&blkdev._dev, VIRTIO_STATUS_FEATURES_OK);

	mb();

	if(!(get_status(&blkdev._dev) & VIRTIO_STATUS_FEATURES_OK))
	{
		printf("HOST REFUSED OUR FEATURES\n");
	}
	else
	{
		printf("HOST is ok with our features\n");
	}
	

//
	uint32_t totSectorCount = ReadReg32(&blkdev._dev, 0x14);
    uint8_t maxSegSize      =  ReadReg8(&blkdev._dev, 0x1C);
    uint8_t maxSegCount     =  ReadReg8(&blkdev._dev, 0x20);
    uint8_t cylinderCount   =  ReadReg8(&blkdev._dev, 0x24);
    uint8_t headCount       =  ReadReg8(&blkdev._dev, 0x26);
    uint8_t sectorCount     =  ReadReg8(&blkdev._dev, 0x27);
    uint8_t blockLen        =  ReadReg8(&blkdev._dev, 0x28);

	printf("Some BLK features\n");
    printf("->totSectorCount %u\n", totSectorCount);
    printf("->maxSegSize %u\n", maxSegSize);
    printf("->maxSegCount %u\n", maxSegCount);
    printf("->cylinderCount %u\n", cylinderCount);
    printf("->headCount %u\n", headCount);
    printf("->sectorCount %u\n", sectorCount);
    printf("->blockLen %u\n", blockLen);

    uint8_t numQueues = 0;
    for(int index = 0;index<16;index++)
    {
        WriteReg16(&blkdev._dev, VIRTIO_PCI_QUEUE_SEL, index);
        uint16_t queueSize = ReadReg16(&blkdev._dev, VIRTIO_PCI_QUEUE_NUM);
        if(queueSize == 0)
        {
            continue;
        }
        numQueues++;
        printf("Queue %i size %i\n",index, queueSize);
    }
    printf("Virtio blk has %i available queues\n", numQueues);




//

	virtq = virtq_create(128);
	assert(virtq);
	printf("virtq_add_to_device\n");
	virtq_add_to_device(virtq, 0);

	add_status(&blkdev._dev, VIRTIO_STATUS_DRIVER_OK);
	mb();
	printf("virtio-blk Status %x\n", get_status(&blkdev._dev));

	blkdev.virtq = virtq;

	void* _data = ps_dma_alloc(&env->ops.dma_manager, 1024, 16, 1, PS_MEM_NORMAL);
	
	assert(_data);
	printf("## TEST CMD\n");
	int err = virtio_blk_cmd(&blkdev, VIRTIO_BLK_T_IN, 10, _data);
	printf("virtio_blk_cmd returned %i\n", err);


	printf("TEST INPUT\n");

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


	seL4_MessageInfo_t msg = seL4_MessageInfo_new(0, 0, 0, 1);
	seL4_Send(irq_aep, msg);
	while(1)
    {

	    seL4_Wait(irq_aep,NULL);
		printf("INPUT\n");
        seL4_IRQHandler_Ack(irq);

		virtio_blk_isr(0);
	}
	
}

static int virtio_blk_cmd(struct virtio_blk *blk, uint32_t type, uint32_t sector, uint8_t *data)
{
	KernelTaskContext* env = getKernelTaskContext();
	
	struct virtio_blk_req *hdr = ps_dma_alloc(&env->ops.dma_manager, VIRTIO_BLK_REQ_HEADER_SIZE + 1, 16, 1, PS_MEM_NORMAL);
	uint32_t d1, d2, d3, datamode = 0;

	hdr->type = type;
	hdr->sector = sector;

	d1 = virtq_alloc_desc(blk->virtq, hdr, VIRTIO_BLK_REQ_HEADER_SIZE);
	blk->virtq->desc[d1].len = VIRTIO_BLK_REQ_HEADER_SIZE;
	blk->virtq->desc[d1].flags = VIRTQ_DESC_F_NEXT;

	if (type == VIRTIO_BLK_T_IN) /* if it's a read */
		datamode = VIRTQ_DESC_F_WRITE; /* mark page writeable */

	d2 = virtq_alloc_desc(blk->virtq, data, VIRTIO_BLK_SECTOR_SIZE);
	blk->virtq->desc[d2].len = VIRTIO_BLK_SECTOR_SIZE;
	blk->virtq->desc[d2].flags = datamode | VIRTQ_DESC_F_NEXT;

	d3 = virtq_alloc_desc(blk->virtq, (void*)hdr + VIRTIO_BLK_REQ_HEADER_SIZE, VIRTIO_BLK_REQ_FOOTER_SIZE);

	blk->virtq->desc[d3].len = VIRTIO_BLK_REQ_FOOTER_SIZE;
	blk->virtq->desc[d3].flags = VIRTQ_DESC_F_WRITE;

	blk->virtq->desc[d1].next = d2;
	blk->virtq->desc[d2].next = d3;

	printf("Block stuff is OK write at %i\n", blk->virtq->avail->idx);
	assert(blk->virtq->avail);
	blk->virtq->avail->ring[blk->virtq->avail->idx] = d1;
	mb();
	blk->virtq->avail->idx += 1;
	mb();
	WriteReg16(&blkdev._dev, VIRTIO_PCI_QUEUE_NOTIFY, 0);// dev->queueID);
	//WRITE32(blk->regs->QueueNotify, 0);
	printf("END CMD\n");
}

static int virtio_blk_read(struct virtio_blk *blk, uint32_t sector, uint8_t *data)
{
	return virtio_blk_cmd(blk, VIRTIO_BLK_T_IN, sector, data);
}

static int virtio_blk_write(struct virtio_blk *blk, uint32_t sector, uint8_t *data)
{
	return virtio_blk_cmd(blk, VIRTIO_BLK_T_OUT, sector, data);
}