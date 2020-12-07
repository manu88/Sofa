#include "IODevice.h"

ssize_t IODeviceRead(IODevice* dev, size_t sector, char* buf, size_t bufSize)
{
    return dev->ops->read(dev, sector, buf, bufSize);
}

ssize_t IODeviceWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize)
{
    return dev->ops->write(dev, sector, buf, bufSize);
}
