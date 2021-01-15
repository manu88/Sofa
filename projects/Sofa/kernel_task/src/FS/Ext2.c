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
#include "Ext2.h"
#include "Log.h"


typedef struct
{
    uint32_t blockID;
    UT_hash_handle hh;
    uint8_t data[];
} Block;

static Block* _blockCache = NULL;

int Ext2ReadBlock(uint8_t *buf, uint32_t blockID, IODevice *dev)
{
    return ext2_read_block(buf, blockID, dev, getExtPriv());
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
#if 0
    Block* b = NULL;
    Block* tmp = NULL;
    HASH_ITER(hh, _blockCache, b, tmp )
    {
        KLOG_DEBUG("\t %u in cache\n", b->blockID);
    }
#endif
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
	//printf("We seek BG %d\n", bg);
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	//printf("Index of our inode is %d\n", index);
	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	//printf("Relative: %d, Absolute: %d\n", block, bgd->block_of_inode_table + block);
    block_buf = Ext2ReadBlockCached(bgd->block_of_inode_table + block, dev);
    if(block_buf == NULL)
    {
        return 0;
    }

	inode_t* _inode = (inode_t *)block_buf;
	index = index % priv->inodes_per_block;
    //printf("Index is %i\n", index);
	for(i = 0; i < index; i++)
	{
		_inode++;
	}
 
	/* We have found the inode! */
    //printf("Found the inode\n");

	memcpy(inode_buf, _inode, sizeof(inode_t));
	return 1;
}