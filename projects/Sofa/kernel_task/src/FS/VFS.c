#include <stdio.h>
#include <string.h>
#include "VFS.h"
#include "ext2.h"


int VFSInit()
{
    return 0;
}

static IODevice* _dev = NULL;
void VFSLs(const char* path)
{
    printf("ls request '%s'\n", path);

    uint8_t ret = ext2_exist(path, _dev, NULL);
    printf("Ret %u\n", ret);
}



static int _VFSCheckSuperBlock(IODevice* dev, VFSSupported* fsType)
{
    assert(fsType);

    if(ext2_probe(dev))
    {
        if(ext2_mount(dev, NULL))
        {
            return 0;
        }
        printf("ext2_mount error for '%s'\n", dev->name);
    }
    else
    {
        printf("ext2_probe error for '%s'\n", dev->name);
    }

    return -1;
}


int VFSAddDEvice(IODevice *dev)
{
    printf("[VFS] add device '%s'\n", dev->name);

    VFSSupported type = VFSSupported_Unknown;
    int test = _VFSCheckSuperBlock(dev, &type);
    if(test != 0)
    {
        return test;
    }
    printf("[VFS] device  '%s' added with FS type %i\n", dev->name, type);
    _dev = dev;
}