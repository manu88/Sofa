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
#include "Log.h"
#include "KThread.h"
#include "Environ.h"
#include "utils.h"


ssize_t IODeviceRead(IODevice* dev, size_t sector, char* buf, size_t bufSize)
{
    return dev->ops->read(dev, sector, buf, bufSize);
}

ssize_t IODeviceReadAsync(IODevice* dev, IODeviceRequest* reply)
{
    return dev->ops->readAsync(dev, reply);
}

ssize_t IODeviceWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize)
{
    return dev->ops->write(dev, sector, buf, bufSize);
}

void IODeviceRequestReply(IODeviceRequest* req, ssize_t ret)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0 ,0, 2);
    seL4_SetMR(1, ret);
    seL4_Send(req->replyCap, info);
}
