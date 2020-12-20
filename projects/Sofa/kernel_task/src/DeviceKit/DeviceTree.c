/*
For a list of VID/PID : https://github.com/openbsd/src/blob/master/sys/dev/pci/pcidevs.h
*/
#include <platsupport/io.h>
#include "DeviceTree.h"
#include "Environ.h"

#include <ctype.h>

#include "X86Expert.h"
#include "Drivers/Net.h"
#include "Drivers/Blk.h"
#include "KThread.h"

static IODevice *_deviceList = NULL;

IODevice* DeviceTreeGetDevices()
{
    return _deviceList;
}

int DeviceTreeAddDevice(IODevice* dev)
{
    DL_APPEND(_deviceList, dev);
    return 0;
}

int DeviceTreeInit()
{   
    int err = PlatformExpertInit();
    if(err != 0)
    {
        return err;
    }
    return PlatformExpertConstructTree();
}