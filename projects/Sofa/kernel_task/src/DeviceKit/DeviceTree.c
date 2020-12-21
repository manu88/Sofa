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


static IODevice *_deviceList = NULL;

static IONode _rootNode = IONodeNew("\\");


const IONode* DeviceTreeGetRoot()
{
    return &_rootNode;
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
            printf("\t");
        }
        printf("'%s' ", c->name);
        printf("HID=");

        if(c->hid.type == IOVariantType_UINT64)
        {
            if(isEisaId(c->hid.value.v))
            {
                char str[8] = "";
                getEisaidString(c->hid.value.v, str);
                printf("%s", str);
            }
            else
            {
                printf("0X%lX", c->hid.value.v);
            }
        }
        else if(c->hid.type == IOVariantType_STRING)
        {
            printf("%s", c->hid.value.s);
        }
        if(c->driver)
        {
            printf(" DRIVER ");
        }
        printf("\n");

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
