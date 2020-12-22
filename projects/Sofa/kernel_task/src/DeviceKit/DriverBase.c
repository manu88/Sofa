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
#include "DriverBase.h"


void WriteReg8(Device *dev, uint16_t port, uint8_t val) 
{
    ps_io_port_out(&dev->ioops, dev->iobase + port, 1, val);
}

uint8_t ReadReg8(Device *dev, uint16_t port) 
{
    uint32_t val;
    ps_io_port_in(&dev->ioops, dev->iobase + port, 1, &val);
    return (uint8_t)val;
}

void WriteReg16(Device *dev, uint16_t port, uint16_t val)
{
    ps_io_port_out(&dev->ioops, dev->iobase + port, 2, val);
}

uint16_t ReadReg16(Device *dev, uint16_t port)
{
    uint32_t val;
    ps_io_port_in(&dev->ioops, dev->iobase + port, 2, &val);
    return (uint16_t)val;
}

void WriteReg32(Device *dev, uint16_t port, uint32_t val)
{
    ps_io_port_out(&dev->ioops, dev->iobase + port, 4, val);
}

uint32_t ReadReg32(Device *dev, uint16_t port) 
{
    uint32_t val;
    ps_io_port_in(&dev->ioops, dev->iobase + port, 4, &val);
    return val;
}