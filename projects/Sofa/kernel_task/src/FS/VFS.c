#include <stdio.h>
#include <string.h>
#include "VFS.h"
#include "ext2.h"
#include "KThread.h"
#include "DeviceTree.h"
#include "NameServer.h"
#include "Environ.h"

static KThread _vfsThread;
static Service _vfsService;
static char _vfsName[] = "VFS";
int VFSInit()
{
    ServiceInit(&_vfsService, getKernelTaskProcess() );
    _vfsService.name = _vfsName;
    NameServerRegister(&_vfsService);
    return 0;
}

static IODevice* _dev = NULL;
void VFSLs(const char* path)
{
    printf("ls request '%s'\n", path);
    uint8_t ret = ext2_read_root_directory(path, _dev, NULL);
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
    _dev = dev;

    VFSSupported type = VFSSupported_Unknown;
    int test = _VFSCheckSuperBlock(dev, &type);
    if(test != 0)
    {
        return test;
    }
    printf("[VFS] device  '%s' added with FS type %i\n", dev->name, type);
}

static int mainVFS(KThread* thread, void *arg)
{
    printf("Test Thread started\n");
    IODevice* dev = NULL;
    FOR_EACH_DEVICE(dev)
    {
        if(dev->type == IODevice_BlockDev)
        {
            VFSAddDEvice(dev);
        }
    }

    while (1)
    {
        /* code */
    }
    
    return 42;
}

int VFSStart()
{
    printf("--> Start VFSD thread\n");
    KThreadInit(&_vfsThread);
    _vfsThread.mainFunction = mainVFS;
    _vfsThread.name = "VFSD";
    int error = KThreadRun(&_vfsThread, 254, NULL);
    if( error != 0)
    {
        return error;
    }
    assert(error == 0);
    printf("<-- Start VFSD thread\n");

    return 0;
}