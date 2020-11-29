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