#include <stdio.h>
#include "VFS.h"
#include "ext2.h"

int VFSInit()
{
    return 0;
}

static int _VFSCheckSuperBlock(IODevice* dev, VFSSupported* fsType)
{
    assert(fsType);
    struct ext2_superblock sb;
    printf("Read super block size is %zi\n", sizeof(struct ext2_superblock));

    ssize_t ret = IODeviceRead(dev, 2, &sb, sizeof(struct ext2_superblock));
    printf("Superblock read %zi\n",ret);
    if(ret <=0)
    {
        printf("UNable to read super block on device '%s' error = %i\n", dev->name, ret);
        return -1;
    }


    printf("'%s'\n", sb.path_last_mounted_to); 

    for (int j=0;j<16;j++)
    {
        printf("%X ", sb.fsid[j]);
    }
    printf("\n"); 


    if(sb.ext2_signature != 0xef53)
    {
        printf("Ext2 signature does'nt match 0XEF53. got %X\n", sb.ext2_signature);
        return -2;
    }
    *fsType = VFSSupported_EXT2;
    return 0;
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

}