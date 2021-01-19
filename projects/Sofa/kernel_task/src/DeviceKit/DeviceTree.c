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
/*
For a list of VID/PID : https://github.com/openbsd/src/blob/master/sys/dev/pci/pcidevs.h
*/
#include <platsupport/io.h>
#include <ctype.h>

#include "DeviceTree.h"
#include "Environ.h"
#include "Log.h"

#include "X86Expert.h"
#include "Drivers/Net.h"
#include "Drivers/Blk.h"
#include "KThread.h"
#include "devFS.h"


static IODevice *_deviceList = NULL;

static IONode _rootNode = IONodeNew("\\");


const IONode* DeviceTreeGetRoot()
{
    return &_rootNode;
}

IODevice* DeviceTreeGetDeviceFromHandle(const void* handle)
{
    if(!handle)
    {
        return NULL;
    }
    IODevice* dev = NULL;
    FOR_EACH_DEVICE(dev)
    {
        if(dev == handle)
        {
            return dev;
        }
    }
    return NULL;
}
IODevice* DeviceTreeGetDevices()
{
    return _deviceList;
}

int DeviceTreeAddDevice(IODevice* dev)
{
    DL_APPEND(_deviceList, dev);
    return 0;
}

static void walkDev(const IONode* n, int indent);

int DeviceTreeInit()
{   
    int err = PlatformExpertInit();
    if(err != 0)
    {
        return err;
    }
    err = PlatformExpertConstructTree(&_rootNode);
    return err;
}


static void walkDev(const IONode* n, int indent)
{
    IONode* c = NULL;
    IONodeForEachChildren(n, c)
    {
        for(int i=0;i<indent;i++)
        {
            KLOG_DEBUG("\t");
        }
        KLOG_DEBUG("'%s' ", c->name);
        KLOG_DEBUG("HID=");

        if(c->hid.type == IOVariantType_UINT64)
        {
            if(isEisaId(c->hid.value.v))
            {
                char str[8] = "";
                getEisaidString(c->hid.value.v, str);
                KLOG_DEBUG("%s", str);
            }
            else
            {
                KLOG_DEBUG("0X%lX", c->hid.value.v);
            }
        }
        else if(c->hid.type == IOVariantType_STRING)
        {
            KLOG_DEBUG("%s", c->hid.value.s);
        }
        if(c->driver)
        {
            KLOG_DEBUG(" DRIVER ");
        }
        if(c->devFile)
        {
            KLOG_DEBUG(" %s ", c->devFile->name);
        }
        KLOG_DEBUG("\n");

        walkDev(c, indent+1);
    }
}

void DeviceTreePrint(const IONode* node)
{
    walkDev(node, 0);
}

static PoweroffMethod _poweroff = NULL;
static void* _powerOffPtr = NULL;
void DeviceTreeHardwarePoweroff()
{
    if(_poweroff)
    {
        _poweroff(_powerOffPtr);
    }
    else
    {
        KLOG_INFO("No way to poweroff the hardware\n");
    }
    
}

int DeviceTreeAddPoweroff(PoweroffMethod method, void* ptr)
{
    _poweroff = method;
    _powerOffPtr = ptr;
    return 0;
}
