#include <stdio.h>
#include <string.h>
#include "VFS.h"
#include "ext2.h"


int VFSInit()
{
    return 0;
}


static void testBlk(IODevice* dev)
{

    uint8_t str[512];// = "Hello this is some data";
    for (int i=0;i<512;i++)
    {
        str[i] = i;
    }
    printf("--- WRITE ---\n");
    IODeviceWrite(dev, 88, str, 512);
    printf("DID Write\n");


    uint8_t b[512];
    memset(b, 0, 512);
    printf("--- READ ---\n");
    ssize_t ret = IODeviceRead(dev, 88, b, 512);
    printf("READ return %zi\n",ret);
    for(int i=0;i<100;i++)
    {
        printf(" %i ", b[i]);
    }
    printf("\n");
}

static int _VFSCheckSuperBlock(IODevice* dev, VFSSupported* fsType)
{
    assert(fsType);
    ext2_superblock_t sb;

    ssize_t ret = IODeviceRead(dev, 2, &sb, sizeof(ext2_superblock_t));
    printf("Superblock size %zi\n" ,sizeof(ext2_superblock_t));
    if(ret <=0)
    {
        printf("UNable to read super block on device '%s' error = %i\n", dev->name, ret);
        return -1;
    }

    printf("Ext2 version %i.%i\n", sb.major_version, sb.minor_version);

    uint32_t blocksize = 1024 << sb.blocksize_hint;
    printf("Block size is %u\n",  blocksize);
    printf("Super block is at %u\n", sb.superblock_id);


    if(sb.ext2_sig != EXT2_SIGNATURE)
    {
        printf("Ext2 signature does'nt match 0XEF53. got %X\n", sb.ext2_sig);
        return -2;
    }
    *fsType = VFSSupported_EXT2;

    uint32_t block_bgdt = sb.superblock_id + (sizeof(ext2_superblock_t) / blocksize);
    printf("block_bgdt %u\n", block_bgdt);

    uint8_t buff[512];
    ret = IODeviceRead(dev, 4, &buff, 512);

    printf("ret for bgdt %zi\n", ret);
    ext2_block_group_desc_t *bgdt = buff;
    printf("Starting block addr %zi \n", bgdt->block_of_inode_table);

    return 0;
}


int VFSAddDEvice(IODevice *dev)
{
    printf("[VFS] add device '%s'\n", dev->name);

    printf("TEST 1\n");
    testBlk(dev);
    return -1;

    VFSSupported type = VFSSupported_Unknown;
    int test = _VFSCheckSuperBlock(dev, &type);
    if(test != 0)
    {
        return test;
    }
        printf("[VFS] device  '%s' added with FS type %i\n", dev->name, type);

}