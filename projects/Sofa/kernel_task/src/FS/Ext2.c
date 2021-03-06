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
/**
 * Implementation inspired by: 
 * Levente Kurusa <levex@linux.com> 
 * From https://github.com/levex/osdev/blob/master/include/ext2.h
 * **/
#include "Ext2.h"
#include "Log.h"


typedef struct
{
    uint32_t blockID;
    UT_hash_handle hh;
    uint8_t data[];
} Block;

static Block* _blockCache = NULL;

static ext2_priv_data __ext2_data;

ext2_priv_data* getExtPriv()
{
	return &__ext2_data;
}

static uint8_t doReadBlock(uint8_t *buf, uint32_t block, IODevice *dev, ext2_priv_data *priv)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block)
    {
        sectors_per_block = 1;
    }

    uint32_t startSect = block*sectors_per_block;
    uint32_t numSectors = block*sectors_per_block + sectors_per_block - startSect;

    buf[(numSectors*512)-1] = 0;

    uint8_t *bufPos = buf;
    size_t acc = 0;
    for(uint32_t i=0;i<numSectors;i++)
    {
        ssize_t ret = IODeviceRead(dev, priv->lbaStart+startSect+i, bufPos, 512);
		if(ret <= 0)
		{
			return 0;// error
		}
        bufPos += ret;
        acc += ret;
    }
	return 1;
}

int Ext2ReadBlock(uint8_t *buf, uint32_t blockID, IODevice *dev)
{
	int r = doReadBlock(buf, blockID, dev, getExtPriv()); 
    if(!r)
	{
		KLOG_DEBUG("Ext2ReadBlock read error, retry once\n");
		return doReadBlock(buf, blockID, dev, getExtPriv());
	}
	return r;
}


uint8_t* Ext2ReadBlockCached(uint32_t blockID, IODevice* dev)
{
    Block* blk = NULL;

    HASH_FIND_INT(_blockCache, &blockID, blk);
    if(blk)
    {
        return blk->data;
    }

    blk = malloc(sizeof(Block) + getExtPriv()->blocksize);
    if(!Ext2ReadBlock(blk->data, blockID, dev))
    {
        free(blk);
        return NULL;
    }
    blk->blockID = blockID;
    HASH_ADD_INT(_blockCache, blockID, blk);
	assert(blk->blockID == blockID);

    return blk->data;
}

uint8_t Ext2ReadInode(inode_t *inode_buf, uint32_t inode, IODevice *dev)
{
    ext2_priv_data *priv = getExtPriv();
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;

    uint8_t *block_buf = Ext2ReadBlockCached(priv->first_bgd, dev);

    if(block_buf == NULL)
    {
        return 0;
    }
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;

	for(i = 0; i < bg; i++)
		bgd++;

	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;

	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;

    block_buf = Ext2ReadBlockCached(bgd->block_of_inode_table + block, dev);
    if(block_buf == NULL)
    {
        return 0;
    }

	inode_t* _inode = (inode_t *)block_buf;
	index = index % priv->inodes_per_block;

	for(i = 0; i < index; i++)
	{
		_inode++;
	}

	memcpy(inode_buf, _inode, sizeof(inode_t));
	return 1;
}

uint8_t Ext2Probe(IODevice *dev, size_t lbaStart)
{
	if(!dev->ops)
	{
		printf("Device has no operations, skipped.\n");
		return 0;
	}
    if(!dev->ops->read)
	{
		printf("Device has no read, skipped.\n");
		return 0;
	}
	uint8_t *buf = (uint8_t *)malloc(1024);
    ssize_t ret = IODeviceRead(dev, lbaStart + 2, buf, 512);
    ret = IODeviceRead(dev, lbaStart + 3, buf+512, 512);

	superblock_t *sb = (superblock_t *)buf;
	if(sb->ext2_sig != EXT2_SIGNATURE)
	{
		printf("Invalid EXT2 signature, have: 0x%x!\n", sb->ext2_sig);
		free(buf);
		return 0;
	}
	printf("Valid EXT2 signature!\n");
	
    ext2_priv_data *priv = getExtPriv();
	priv->lbaStart = lbaStart;

	memcpy(&priv->sb, sb, sizeof(superblock_t));
	/* Calculate volume length */
	uint32_t blocksize = 1024 << sb->blocksize_hint;
	printf("Size of a block: %d bytes\n", blocksize);
	priv->blocksize = blocksize;
	priv->inodes_per_block = blocksize / sizeof(inode_t);
	priv->sectors_per_block = blocksize / 512;
	printf("Size of volume: %d bytes\n", blocksize*(sb->blocks));
	/* Calculate the number of block groups */
	uint32_t number_of_bgs0 = sb->blocks / sb->blocks_in_blockgroup;
	if(!number_of_bgs0) number_of_bgs0 = 1;
	printf("There are %d block group(s).\n", number_of_bgs0);
	priv->number_of_bgs = number_of_bgs0;
	/* Now, we have the size of a block,
	 * calculate the location of the Block Group Descriptor
	 * The BGDT is located directly after the SB, so obtain the
	 * block of the SB first. This is located in the SB.
	 */
	uint32_t block_bgdt = sb->superblock_id + (sizeof(superblock_t) / blocksize);
	priv->first_bgd = 1;//block_bgdt;
    printf("first_bgd is at %u\n", priv->first_bgd);
    

	printf("Device %s is with EXT2 filesystem. Probe successful.\n", dev->name);
	free(buf);

	return 1;
}

uint8_t Ext2Mount(IODevice *dev)
{
	printf("Mounting ext2 on device %s\n", dev->name);

	inode_t ino;
	if(Ext2ReadInode(&ino, 2, dev) == 0)
	{
		return 0;
	}

	if((ino.type & 0xF000) != INODE_TYPE_DIRECTORY)
	{
		printf("FATAL: Root directory is not a directory!\n");
		return 0;
	}


	return 1;
}