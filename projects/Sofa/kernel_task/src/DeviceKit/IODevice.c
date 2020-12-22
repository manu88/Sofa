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
#include "IODevice.h"

ssize_t IODeviceRead(IODevice* dev, size_t sector, char* buf, size_t bufSize)
{
    return dev->ops->read(dev, sector, buf, bufSize);
}

ssize_t IODeviceWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize)
{
    return dev->ops->write(dev, sector, buf, bufSize);
}
