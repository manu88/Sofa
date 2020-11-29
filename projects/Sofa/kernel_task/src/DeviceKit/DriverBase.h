#pragma once
#include <platsupport/io.h>
#include <sys/types.h>



#define VIRTIO_PCI_HOST_FEATURES	0
/* An 8-bit device status register.  */

/* A 32-bit r/w bitmask of features activated by the guest */
#define VIRTIO_PCI_GUEST_FEATURES	4


#define VIRTIO_PCI_STATUS		18

typedef struct _Device 
{
	uint32_t iobase;
	ps_io_port_ops_t ioops;

} Device;



uint8_t ReadReg8(Device *dev, uint16_t port);
void WriteReg8(Device *dev, uint16_t port, uint8_t val);
uint32_t ReadReg32(Device *dev, uint16_t port);
void WriteReg32(Device *dev, uint16_t port, uint32_t val);
uint16_t ReadReg16(Device *dev, uint16_t port);
void WriteReg16(Device *dev, uint16_t port, uint16_t val);