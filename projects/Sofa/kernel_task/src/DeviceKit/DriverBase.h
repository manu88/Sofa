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