#include <stdio.h>
#include <string.h>
#include "VFS.h"
#include "ext2.h"


int VFSInit()
{
    return 0;
}


static void testBlk(IODevice* dev, int sector)
{
/*
    uint8_t str[512];// = "Hello this is some data";
    for (int i=0;i<512;i++)
    {
        str[i] = i;
    }
    printf("--- WRITE ---\n");
    IODeviceWrite(dev, 2, str, 512);
    printf("DID Write\n");

*/
    uint8_t b[512];
    memset(b, 0, 512);
    printf("--- READ ---\n");
    ssize_t ret = IODeviceRead(dev, sector, b, 512);
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



    printf("inodes %u\n", sb.inodes);
    printf("blocks %u\n", sb.blocks);
    printf("reserved_for_root %u\n", sb.reserved_for_root);
    printf("unallocatedblocks %u\n", sb.unallocatedblocks);
    printf("unallocatedinodes %u\n", sb.unallocatedinodes);
    printf("superblock_id %u\n", sb.superblock_id);
    printf("blocksize_hint %u\n", sb.blocksize_hint);
    printf("fragmentsize_hint %u\n", sb.fragmentsize_hint);
    printf("blocks_in_blockgroup %u\n", sb.blocks_in_blockgroup);
    printf("frags_in_blockgroup %u\n", sb.frags_in_blockgroup);
    printf("inodes_in_blockgroup %u\n", sb.inodes_in_blockgroup);
    printf("last_mount %u\n", sb.last_mount);
    printf("last_write %u\n", sb.last_write);
    printf("mounts_since_last_check %u\n", sb.mounts_since_last_check);
    printf("max_mounts_since_last_check %u\n", sb.max_mounts_since_last_check);
    printf("ext2_sig %u\n", sb.ext2_sig);
    printf("state %u\n", sb.state);
    printf("op_on_err %u\n", sb.op_on_err);
    printf("minor_version %u\n", sb.minor_version);
    printf("last_check %u\n", sb.last_check);
    printf("max_time_in_checks %u\n", sb.max_time_in_checks);
    printf("os_id %u\n", sb.os_id);
    printf("major_version %u\n", sb.major_version);
    printf("uuid %u\n", sb.uuid);
    printf("gid %u\n", sb.gid);
    printf("Ext2 version %i.%i\n", sb.major_version, sb.minor_version);

    uint32_t blocksize = 1024 << sb.blocksize_hint;
    printf("Block size is %u\n",  blocksize);
    printf("Super block is at %u\n", sb.superblock_id);
    printf("Inodes per block group %i\n", sb.inodes_in_blockgroup);

    uint32_t sectorPerBlock = blocksize / 512;
    printf("Sectors per block = %u\n", sectorPerBlock);

    if(sb.ext2_sig != EXT2_SIGNATURE)
    {
        printf("Ext2 signature does'nt match 0XEF53. got %X\n", sb.ext2_sig);
        return -2;
    }
    *fsType = VFSSupported_EXT2;

    uint32_t block_bgdt = sb.superblock_id + (sizeof(ext2_superblock_t) / blocksize);
    printf("block_bgdt %u\n", block_bgdt);

    uint8_t buff[512];
    const uint32_t block_bgdt_sector = sectorPerBlock * (block_bgdt+1);
    ret = IODeviceRead(dev, block_bgdt_sector, &buff, 512);

    printf("ret for bgdt %zi\n", ret);
    ext2_block_group_desc_t *bgdt = buff;
    printf("Starting block addr %zi \n", bgdt->block_of_inode_table);
    printf("Num of dirs %i\n", bgdt->num_of_dirs);

    printf("block_of_block_usage_bitmap %u\n", bgdt->block_of_block_usage_bitmap);
    printf("block_of_inode_usage_bitmap %u\n", bgdt->block_of_inode_usage_bitmap);
    printf("block_of_inode_table %u\n", bgdt->block_of_inode_table);
    printf("num_of_unalloc_block %u\n", bgdt->num_of_unalloc_block);
    printf("num_of_unalloc_inode %u\n", bgdt->num_of_unalloc_inode);


    int rootInode = 2; //Always
    int rootGroupIndex = (rootInode -1) % sb.inodes_in_blockgroup;
    printf("rootGroupIndex %i\n", rootGroupIndex);
    uint32_t inodes_per_block = blocksize / sizeof(inode_t);
    printf("inodes_per_block %i\n", inodes_per_block);

    uint32_t index = (rootInode - 1) % sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(inode_t)) / blocksize;
	index = index % inodes_per_block; // relative to bgdt->block_of_inode_table
    uint32_t absIndex = bgdt->block_of_inode_table + index;

    printf("index %i\n", index);
    printf("absIndex %i\n", absIndex);

    assert(0);
    uint8_t rootInodeData[512] = "";

    uint32_t startSector = 2;
    for(int j=0;j<50;j++)
    {
        

        uint32_t sector = startSector +j;

        ret = IODeviceRead(dev, sector, rootInodeData, 512);
        assert(ret);
        printf("Did read root inode sector %i\n",sector);

        for(int i=0;i<4;i++)
        {
            inode_t* inode = rootInodeData + i*sizeof(inode_t);
            if((inode->type & 0xF000) == INODE_TYPE_DIRECTORY)
            {
                printf("Looks like a directory at %i-%i\n", sector,i);
                for(int i=0;i<12;i++)
                {
                    printf("dbp %i %u\n", i, inode->dbp[i]);
                }
            }
        }

        //printf("TYPE %X\n", inode->type);
    }
    return 0;
}


int VFSAddDEvice(IODevice *dev)
{
    printf("[VFS] add device '%s'\n", dev->name);
/*
    for(int i=0;i<30;i++)
    {
        printf("TEST %i\n", i);
        testBlk(dev, i);
    }
    return -1;
*/
    VFSSupported type = VFSSupported_Unknown;
    int test = _VFSCheckSuperBlock(dev, &type);
    if(test != 0)
    {
        return test;
    }
        printf("[VFS] device  '%s' added with FS type %i\n", dev->name, type);

}