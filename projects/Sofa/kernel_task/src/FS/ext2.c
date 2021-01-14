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
/** @author Levente Kurusa <levex@linux.com> **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ext2.h"
#include "IODevice.h"
#include "Log.h"


static size_t strsplit(char* str, char delim)
{
	size_t n = 0;
	uint32_t i = 0;
	while(str[i])
	{
		if(str[i] == delim)
		{
			str[i] = 0;
			n++;
		}
		i++;
	}
	n++;
	return n;
}

static inode_t *inode = 0;
static uint8_t *root_buf = 0;
static uint8_t *block_buf = 0;


static ext2_priv_data __ext2_data;

ext2_priv_data* getExtPriv()
{
	return &__ext2_data;
}

typedef IODevice device_t;

uint8_t ext2_read_block(uint8_t *buf, uint32_t block, device_t *dev, ext2_priv_data *priv)
{
	if(priv == NULL)
	{
		priv = &__ext2_data;
	}
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block)
    {
        sectors_per_block = 1;
    }

    uint32_t startSect = block*sectors_per_block;
    uint32_t numSectors = block*sectors_per_block + sectors_per_block - startSect;
	//printf("we want to read block %d which is sectors [%d; %d]\n", block, startSect , numSectors);

    //printf("Input buffer should have size %zi\n", numSectors*512);
    buf[(numSectors*512)-1] = 0;
    //printf("Test touch input buffer OK\n");


    uint8_t *bufPos = buf;
    size_t acc = 0;
    for(uint32_t i=0;i<numSectors;i++)
    {
		//printf("Read sector %i\n", startSect+i);
        ssize_t ret = IODeviceRead(dev, startSect+i, bufPos, 512);
		if(ret <= 0)
		{
			return 0;// error
		}
        bufPos += ret;
        acc += ret;
        //printf("Buf pos %zi\n", acc);
    }
	return 1;
}

void ext2_write_block(uint8_t *buf, uint32_t block, device_t *dev, ext2_priv_data *priv)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;

    assert(0 && "Implement me :)");
}

uint8_t ext2_read_inode(inode_t *inode_buf, uint32_t inode, device_t *dev, ext2_priv_data *priv)
{
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	if(!block_buf) 
    {
        block_buf = (uint8_t *)malloc(priv->blocksize);
    }
    assert(block_buf);

	uint8_t ret = ext2_read_block(block_buf, priv->first_bgd, dev, priv);
	if(ret == 0)
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
	ret = ext2_read_block(block_buf, bgd->block_of_inode_table + block, dev, priv);
	if(ret == 0)
	{
		return 0;
	}
    //printf("Did read\n");
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

void ext2_write_inode(inode_t *inode_buf, uint32_t ii, device_t *dev, ext2_priv_data *priv)
{
	uint32_t bg = (ii - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	if(!block_buf) block_buf = (uint8_t *)malloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	//printf("We seek BG %d\n", bg);
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (ii - 1) % priv->sb.inodes_in_blockgroup;
	//printf("Index of our inode is %d\n", index);
	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	//printf("Relative: %d, Absolute: %d\n", block, bgd->block_of_inode_table + block);
	uint32_t final = bgd->block_of_inode_table + block;
	ext2_read_block(block_buf, final, dev, priv);
	inode_t* _inode = (inode_t *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	memcpy(_inode, inode_buf, sizeof(inode_t));
	ext2_write_block(block_buf, final, dev, priv);
}

uint32_t ext2_get_inode_block(uint32_t inode, uint32_t *b, uint32_t *ioff, device_t *dev, ext2_priv_data *priv)
{
	if(priv == NULL)
	{
		priv = &__ext2_data;
	}
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	if(!block_buf) block_buf = (uint8_t *)malloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	//printf("We seek BG %d\n", bg);
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	index = index % priv->inodes_per_block;
	*b = block + bgd->block_of_inode_table;
	*ioff = index;
	return 1;
}

uint32_t ext2_read_directory(char *filename, ext2_dir *dir, device_t *dev, ext2_priv_data *priv)
{
	KLOG_DEBUG("ext2_read_directory for '%s'\n", filename);
	if(priv == NULL)
	{
		priv == &__ext2_data;
	}
    assert(filename);
    //printf("ext2_read_directory\n");
	while(dir->inode != 0) 
    {
		char *name = (char *)malloc(dir->namelength + 1);
        assert(name);
		memcpy(name, &dir->reserved+1, dir->namelength);
		name[dir->namelength] = 0;
	    printf("ENTRY: %s (%d) ino=%u\n", name, dir->size, dir->inode);
        //printf("Compare with '%s'\n", filename);
		if(filename && strcmp(filename, name) == 0)
		{
			/* If we are looking for a file, we had found it */
			uint8_t r = ext2_read_inode(inode, dir->inode, dev, priv);
			if(r == 0)
			{
				free(name);
				return 0;
			}
			printf("Found inode %s! %d\n", filename, dir->inode);
			free(name);
			return dir->inode;
		}
		if(!filename && (uint32_t)filename != 1) 
		{
			//printf("Found dir entry: %s to inode %d \n", name, dir->inode);
			printf("%s\n", name);
		}
		dir = (ext2_dir *)((uint32_t)dir + dir->size);
		free(name);
	}
	return 0;
}

uint8_t ext2_read_root_directory(char *filename, device_t *dev, ext2_priv_data *priv)
{
	if(priv == NULL)
	{
		priv = &__ext2_data;
	}

	/* The root directory is always inode#2, so find BG and read the block. */
	if(!inode)
    {
        inode = (inode_t *)malloc(sizeof(inode_t));
    }
	if(!root_buf)
    { 
        root_buf = (uint8_t *)malloc(priv->blocksize);
    }
	if(ext2_read_inode(inode, 2, dev, priv) == 0)
	{
		return 0;
	}

	if((inode->type & 0xF000) != INODE_TYPE_DIRECTORY)
	{
		printf("FATAL: Root directory is not a directory!\n");
		return 0;
	}
	/* We have found the directory!
	 * Now, load the starting block
	 */
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(b == 0)
        {
            break;
        }
        //printf("ext2_read_root_directory: read index %i\n", i);
		ext2_read_block(root_buf, b, dev, priv);
		//printf("did ext2_read_block\n");
		/* Now loop through the entries of the directory */
		if(ext2_read_directory(filename, (ext2_dir*)root_buf, dev, priv))
        {
            //printf("ext2_read_directory OK\n");
            return 1;
        }
	}
	if(filename && (uint32_t)filename != 1)
	{
		return 0;
	}
	return 1;
}

uint8_t ext2_find_file_inode(char *ff, inode_t *inode_buf, device_t *dev, ext2_priv_data *priv)
{
	if(priv == NULL)
	{
		priv == &__ext2_data;
	}
	char *filename = malloc(strlen(ff) + 1);
	memcpy(filename, ff, strlen(ff) +1);
	size_t n = strsplit(filename, '/');
//	filename ++; // skip the first crap
	uint32_t retnode = 0;
	if(n > 1)
	{ 
		/* Read inode#2 (Root dir) into inode */
		if(ext2_read_inode(inode, 2, dev, priv) == 0)
		{
			return 0;
		}
		/* Now, loop through the DPB's and see if it contains this filename */

		n--;
		while(n--)
		{
			printf("Looking for: %s\n", filename);
			for(int i = 0; i < 12; i++)
			{
				uint32_t b = inode->dbp[i];
				if(!b)
				{
					break;
				}
				ext2_read_block(root_buf, b, dev, priv);
				uint32_t rc = ext2_read_directory(filename, (ext2_dir *)root_buf, dev, priv);
				if(!rc)
				{
					if(strcmp(filename, "") == 0)
					{
						free(filename);
						return strcmp(ff, "/")?retnode:1;
					}
					printf("File (%s (0x%x)) not found!\n", filename, filename);
					free(filename);
					return 0;
				} else {
					/* inode now contains that inode
					 * get out of the for loop and continue traversing
					 */
					 retnode = rc;
					 goto fix;
				}
			}
			fix:;
			uint32_t s = strlen(filename);
			filename += s + 1;
		}
		memcpy(inode_buf, inode, sizeof(inode_t));
	}
	else
	{
		/* This means the file is in the root directory */
		ext2_read_root_directory(filename, dev, priv);
		memcpy(inode_buf, inode, sizeof(inode_t));
	}
	free(filename);
	return retnode;
}

void ext2_list_directory(char *dd, char *buffer, device_t *dev, ext2_priv_data *priv)
{
	if(priv == NULL)
	{
		priv = &__ext2_data;
	}
	char *dir = dd;
	int rc = ext2_find_file_inode(dir, (inode_t *)buffer, dev, priv);
	if(!rc)
	{
		return;
	}
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(!b) 
		{	
			break;
		}
		ext2_read_block(root_buf, b, dev, priv);
		ext2_read_directory(0, (ext2_dir *)root_buf, dev, priv);
	}
}
#define SIZE_OF_SINGLY (priv->blocksize * priv->blocksize / 4)
uint8_t ext2_read_singly_linked(uint32_t blockid, uint8_t *buf, device_t *dev, ext2_priv_data *priv)
{
	uint32_t blockadded = 0;
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	/* A singly linked block is essentially an array of
	 * uint32_t's storing the block's id which points to data
	 */
	 /* Read the block into root_buf */
	 ext2_read_block(root_buf, blockid, dev, priv);
	 /* Loop through the block id's reading them into the appropriate buffer */
	 uint32_t *block = (uint32_t *)root_buf;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	/* If it is zero, we have finished loading. */
	 	if(block[i] == 0) break;
	 	/* Else, read the block into the buffer */
	 	ext2_read_block(buf + i * priv->blocksize, block[i], dev, priv);
	 }
	 return 1;
}

uint8_t ext2_read_doubly_linked(uint32_t blockid, uint8_t *buf, device_t *dev, ext2_priv_data *priv)
{
	uint32_t blockadded = 0;
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	/* A singly linked block is essentially an array of
	 * uint32_t's storing the block's id which points to data
	 */
	 /* Read the block into root_buf */
	 ext2_read_block(block_buf, blockid, dev, priv);
	 /* Loop through the block id's reading them into the appropriate buffer */
	 uint32_t *block = (uint32_t *)block_buf;
	 uint32_t s = SIZE_OF_SINGLY;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	/* If it is zero, we have finished loading. */
	 	if(block[i] == 0) break;
	 	/* Else, read the block into the buffer */
	 	ext2_read_singly_linked(block[i], buf + i * s , dev, priv);
	 }
	 return 1;
}

static inode_t *minode = 0;
uint8_t ext2_read_file(char *fn, char *buffer, device_t *dev, ext2_priv_data *priv)
{
	/* Put the file's inode to the buffer */
	if(!minode) minode = (inode_t *)malloc(sizeof(inode_t));
	char *filename = fn;
	if(!ext2_find_file_inode(filename, minode, dev, priv))
	{
		printf("File inode not found.\n");
		return 0;
	}
	for(int i = 0; i < 12; i++)
	{
		uint32_t b = minode->dbp[i];
		if(b == 0) { printf("EOF\n"); return 1; }
		if(b > priv->sb.blocks) 
        {
            Panic("%s: block %d outside range (max: %d)!\n", __func__, b, priv->sb.blocks);
        }
		printf("Reading block: %d\n", b);

		ext2_read_block(root_buf, b, dev, priv);
		//printf("Copying to: 0x%x size: %d bytes\n", buffer + i*(priv->blocksize), priv->blocksize);
		memcpy(buffer + i*(priv->blocksize), root_buf, priv->blocksize);
		//printf("%c%c%c\n", *(uint8_t*)(buffer + 1),*(uint8_t*)(buffer + 2), *(uint8_t*)(buffer + 3));
	}
	if(minode->singly_block) {
		//printf("Block of singly: %d\n", minode->singly_block);
		ext2_read_singly_linked(minode->singly_block, buffer + 12*(priv->blocksize), dev, priv);
	}
	if(minode->doubly_block) {
		uint32_t s = SIZE_OF_SINGLY + 12*priv->blocksize;
		//printf("s is 0x%x (%d)\n", s, s);
		ext2_read_doubly_linked(minode->doubly_block, buffer + s, dev, priv);
	}
	//printf("Read all 12 DBP(s)! *BUG*\n");
	return 1;
}

void ext2_find_new_inode_id(uint32_t *id, device_t *dev, ext2_priv_data *priv)
{
	/* Algorithm: Loop through the block group descriptors,
	 * and find the number of unalloc inodes
	 */

	/* Loop through the block groups */
	ext2_read_block(root_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bg = (block_group_desc_t *)root_buf;
	for(int i = 0; i < priv->number_of_bgs; i++)
	{
		if(bg->num_of_unalloc_inode)
		{
			/* If the bg has some unallocated inodes,
			 * find which inode is unallocated
			 * This is easy:
			 * For each bg we have sb->inodes_in_blockgroup inodes,
			 * this one has num_of_unalloc_inode inodes unallocated,
			 * therefore the latest id is:
			 */
			 *id = ((i + 1) * priv->sb.inodes_in_blockgroup) - bg->num_of_unalloc_inode + 1;
			 bg->num_of_unalloc_inode --;
			 ext2_write_block(root_buf, priv->first_bgd + i, dev, priv);
			 /* Now, update the superblock as well */
			 ext2_read_block(root_buf, priv->sb.superblock_id, dev, priv);
			 superblock_t *sb = (superblock_t *)root_buf;
			 sb->unallocatedinodes --;
			 ext2_write_block(root_buf, priv->sb.superblock_id, dev, priv);
			 return;
		}
		bg++;
	}
}

void ext2_alloc_block(uint32_t *out, device_t *dev, ext2_priv_data *priv)
{
	/* Algorithm: Loop through block group descriptors,
	 * find which bg has a free block
	 * and set that.
	 */
	 ext2_read_block(root_buf, priv->first_bgd, dev, priv);
	 block_group_desc_t *bg = (block_group_desc_t *)root_buf;
	 for(int i = 0; i < priv->number_of_bgs; i++)
	 {
	 	if(bg->num_of_unalloc_block)
	 	{
	 		*out = priv->sb.blocks - bg->num_of_unalloc_block + 1;
	 		bg->num_of_unalloc_block --;
	 		ext2_write_block(root_buf, priv->first_bgd + i, dev, priv);

	 		printf("Allocated block %d\n",*out);

	 		ext2_read_block(root_buf, priv->sb.superblock_id, dev, priv);
			superblock_t *sb = (superblock_t *)root_buf;
			sb->unallocatedblocks --;
			ext2_write_block(root_buf, priv->sb.superblock_id, dev, priv);
			return;
	 	}
	 	bg++;
	 }
}

uint8_t ext2_touch(char *file, device_t *dev, ext2_priv_data *priv)
{
    assert(0 && "implement me :)");
#if 0
	if(!dev->write)
		return 0;
	/* file = "/levex.txt"; */
	/* First create the inode */
	char *fil = (char *)malloc(strlen(file) + 1);
	memcpy(fil, file, strlen(file) + 1);
	inode_t *fi = (inode_t *)malloc(sizeof(inode_t));
	fi->hardlinks = 1;
	fi->size = 0;
	fi->type = INODE_TYPE_FILE;
	fi->disk_sectors = 2;
	/* Create the directory entry */
	size_t n = strsplit(fil, '/');
	n--;
	while(n)
	{
		fil += strlen(fil) + 1;
		n--;
	}
	//printf("filename: %s\n", fil);
	ext2_dir *entry = (ext2_dir *)malloc(sizeof(ext2_dir) + strlen(fil) + 1);
	entry->size = sizeof(ext2_dir) + strlen(fil) + 1;
	entry->namelength = strlen(fil) + 1;
	entry->reserved = 0;
	memcpy(&entry->reserved + 1, fil, strlen(fil) + 1);
	//printf("Length of dir entry: %d + %d = %d\n", sizeof(ext2_dir), strlen(fil), entry->size);
	/* Now, calculate this inode's id,
	 * this is done from the superblock's inodes field
	 * don't forget to update the superblock as well.
	 */
	uint32_t id = 0;
	ext2_find_new_inode_id(&id, dev, priv);
	//printf("Inode id = %d\n", id);
	entry->inode = id;
	//ext2_read_directory(0, entry, dev, priv);
	/* Find where the previous inode is 
	 * and put this inode after this
	 */
	uint32_t block = 0; /* The block where this inode should be written */
	uint32_t ioff = 0; /* Offset into the block function to sizeof(inode_t) */
	ext2_get_inode_block(id, &block, &ioff, dev, priv);
	//printf("This inode is located on block %d with ioff %d\n", block, ioff);
	/* First, read the block in */
	ext2_read_block(root_buf, block, dev, priv);
	inode_t *winode = (inode_t *)root_buf;
	for(int i = 0;i < ioff; i++)
		winode++;
	memcpy(winode, fi, sizeof(inode_t));
	ext2_write_block(root_buf, block, dev, priv);
	/* Now, we have added the inode, write the superblock as well. */
	//ext2_write_block(&priv->sb, priv->sb.superblock_id, dev, priv);
	/* Now, add the directory entry,
	 * for this we have to locate the directory that holds us,
	 * and find his inode. 
	 * We call ext2_find_file_inode() to place the inode to inode_buf
	 */
	char *f = (char *)malloc(strlen(file) + 1);
	memcpy(f, file, strlen(file) + 1);
	str_backspace(f, '/');

	//printf("LF: %s\n", f);
	if(!inode) inode = (inode_t *)malloc(sizeof(inode_t));
	if(!block_buf) block_buf = (uint8_t *)malloc(priv->blocksize);
	uint32_t t = ext2_find_file_inode(f, inode, dev, priv);
	t++;
	//printf("Parent is inode %d\n", t);
	uint8_t found = 0;
	for(int i = 0; i < 12; i++)
	{
		/* Loop through the dpb to find an empty spot */
		if(inode->dbp[i] == 0)
		{
			/* This means we have not yet found a place for our entry,
			 * and the inode has no block allocated.
			 * Allocate a new block for this inode and place it there.
			 */
			uint32_t theblock = 0;
			ext2_alloc_block(&theblock, dev, priv);
			inode->dbp[i] = theblock;
			ext2_write_inode(inode, t, dev, priv);
 		}
		/* This DBP points to an array of directory entries */
		ext2_read_block(block_buf, inode->dbp[i], dev, priv);
		/* Loop throught the directory entries */
		ext2_dir *d = (ext2_dir *)block_buf;
		uint32_t passed = 0;
		while(d->inode != 0) {
			if(d->size == 0) break;
			uint32_t truesize = d->namelength + 8;
			//printf("true size has modulo 4 of %d, adding %d\n", truesize % 4, 4 - truesize%4);
			truesize += 4 - truesize % 4;
			uint32_t origsize = d->size;
			//printf("Truesize: %d Origsize: %d\n", truesize, origsize);
			if(truesize != d->size)
			{
				/* This is the last entry. Adjust the size to make space for our
				 * ext2_dir! Also, note that according to ext2-doc, entries must be on
				 * 4 byte boundaries!
				 */
				d->size = truesize;
				//printf("Adjusted entry len:%d, name %s!\n", d->size, &d->reserved + 1);
				/* Now, skip to the next */
				passed += d->size;
				d = (ext2_dir *)((uint32_t)d + d->size);
				/* Adjust size */
				entry->size = priv->blocksize - passed;
				//printf("Entry size is now %d\n", entry->size);
				break;
			}
			//printf("skipped len: %d, name:%s!\n", d->size, &d->reserved + 1);
			passed += d->size;
			d = (ext2_dir *)((uint32_t)d + d->size);
		}
		/* There is a problem, however. The last entry will always span the whole
		 * block. We have to check if its size field is bigger than what it really is.
		 * If it is, adjust its size, and add the entry after it, adjust our size
		 * to span the block fully. If not, continue as we did before to the next DBP.
		 */

		if(passed >= priv->blocksize)
		{
			//printf("Couldn't find it in DBP %d (%d > %d)\n", i, passed, priv->blocksize);
			continue;
		}
		/* Well, found a free entry! */
		//d = (ext2_dir *)((uint32_t)d + d->size);
	dir_write:
		memcpy(d, entry, entry->size);
		ext2_write_block(block_buf, inode->dbp[i], dev, priv);
		//printf("Wrote to %d\n", inode->dbp[i]);
		return 1;
	}
#endif
	//printf("Couldn't write.\n");
	return 0;
}

uint8_t ext2_writefile(char *fn, char *buf, uint32_t len, device_t *dev, ext2_priv_data *priv)
{
	/* Steps to write to a file:
	 * - Locate and load the inode
	 * - Check if it is of type INODE_TYPE_FILE
	 * -- If no, bail out.
	 * - If inode->size == 0
	 * -- Allocate len / priv->blocksize amount of blocks.
	 * --- Write the buf to the blocks.
	 * - Else, check which block has the last byte, by
	 *   dividing inode->size by priv->blocksize.
	 * -- Load that block.
	 * -- Inside, the last byte is (inode->size)%priv->blocksize
	 * -- If len < priv->blocksize - (inode->size)%priv->blocksize
	 *    which means that the buf can fill the block.
	 * --- Write and return noerror.
	 * -- Else,
	 * --- Write the maximum possible bytes to the block.
	 * --- The next block doesn't exist. Allocate a new one.
	 * --- Write the rest to that block and repeat.
	 * ALSO, on write: adjust inode->size !!!
	 *
	 */

	/* Locate and load the inode */
	uint32_t inode_id = ext2_find_file_inode(fn, inode, dev, priv);
	inode_id ++;
	if(inode_id == 1)
	{
		return 0;
	}
	printf("%s's inode is %d\n", fn, inode_id);
	if(!inode) inode = (inode_t *)malloc(sizeof(inode_t));
	if(ext2_read_inode(inode, inode_id, dev, priv) == 0)
	{
		return 0;
	}
	/* Check if it is of type INODE_TYPE_FILE */
	/*if(! (inode->type & INODE_TYPE_FILE))
	{
		/* -- If no, bail out. 
		printf("Not a file.\n");
		return 0;
	}*/
	/* If inode->size == 0 */
	if(inode->size == 0)
	{
		/* Allocate len / priv->blocksize amount of blocks. */
		uint32_t blocks_to_alloc = len / priv->blocksize;
		blocks_to_alloc ++; /* Allocate atleast one! */
		if(blocks_to_alloc > 12)
		{
			/* @todo */
			printf("Sorry, can't write to files bigger than 12Kb :(\n");
			return 0;
		}
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			uint32_t bid = 0;
			ext2_alloc_block(&bid, dev, priv);
			inode->dbp[i] = bid;
		}
		printf("Allocated %d blocks!\n", blocks_to_alloc);
		inode->size += len; // UPDATE the size
		/* Commit the inode to the disk */
		ext2_write_inode(inode, inode_id - 1, dev, priv);
		/* Write the buf to the blocks. */
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			/* We loop through the blocks and write. */
			ext2_read_block(root_buf, inode->dbp[i], dev, priv);
			if(i + 1 < blocks_to_alloc) { // If not last block
				memcpy(root_buf, buf + i*priv->blocksize, priv->blocksize);
			} else {// If last block
				printf("Last block write %d => %d!\n", i, inode->dbp[i]);
				memcpy(root_buf, buf + i*priv->blocksize, len);
			}
			ext2_write_block(root_buf, inode->dbp[i], dev, priv);
		}
		printf("Wrote the data to fresh blocks!\n");
		return 1;
	}
	/* Else, check which block has the last byte, by
	 *   dividing inode->size by priv->blocksize.
	 */
	uint32_t last_data_block = inode->size / priv->blocksize;
	uint32_t last_data_offset = (inode->size) % priv->blocksize;
	/* Load that block. */
	ext2_read_block(root_buf, last_data_block, dev, priv);
	/* If len < priv->blocksize - (inode->size)%priv->blocksize
	 */
	if(len < priv->blocksize - last_data_offset)
	{
		/*    which means that the buf can fill the block. */
		/* Write and return noerror.*/
		memcpy(root_buf + last_data_offset, buf, len);
		ext2_write_block(root_buf, last_data_block, dev, priv);
		return 1;
	}
	/*Else,
	 * --- Write the maximum possible bytes to the block.
	 * --- The next block doesn't exist. Allocate a new one.
	 * --- Write the rest to that block and repeat.
	 */
	/*uint32_t data_wrote = 0;
	memcpy(root_buf + last_data_offset, buf, priv->blocksize - last_data_offset);
	data_wrote += priv->blocksize - last_data_offset;*/

 	return 0;
}

uint8_t ext2_exist(char *file, device_t *dev, ext2_priv_data *priv)
{
	if(priv == NULL)
	{
		priv = &__ext2_data;
	}
	return ext2_read_file(file, 0, dev, priv);
}

uint8_t ext2_probe(device_t *dev)
{
	/* Read in supposed superblock location and check sig */
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
    IODeviceRead(dev, 2, buf, 512);
    IODeviceRead(dev, 3, buf+512, 512);

	superblock_t *sb = (superblock_t *)buf;
	if(sb->ext2_sig != EXT2_SIGNATURE)
	{
		printf("Invalid EXT2 signature, have: 0x%x!\n", sb->ext2_sig);
		return 0;
	}
	printf("Valid EXT2 signature!\n");
	//filesystem_t *fs = (filesystem_t *)malloc(sizeof(filesystem_t));
	ext2_priv_data *priv = &__ext2_data;// (ext2_priv_data *)malloc(sizeof(ext2_priv_data));
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
    
/*
	fs->name = "EXT2";
	fs->probe = (uint8_t(*)(device_t*)) ext2_probe;
	fs->mount = (uint8_t(*)(device_t*, void *)) ext2_mount;
	fs->read = (uint8_t(*)(char *, char *, device_t *, void *)) ext2_read_file;
	fs->exist = (uint8_t(*)(char *, device_t*, void *)) ext2_exist;
	fs->read_dir = (uint8_t(*)(char * , char *, device_t *, void *)) ext2_list_directory;
	fs->touch = (uint8_t(*)(char *, device_t *, void *)) ext2_touch;
	fs->writefile = (uint8_t(*)(char *, char *m, uint32_t, device_t *, void *)) ext2_writefile;
	fs->priv_data = (void *)priv;
*/
	//dev->fs = fs;
	printf("Device %s is with EXT2 filesystem. Probe successful.\n", dev->name);
	free(buf);
	//free(buffer);
	return 1;
}
uint8_t ext2_mount(device_t *dev, void *privd)
{
    if(privd == NULL)
    {
        privd = &__ext2_data;
    }
	printf("Mounting ext2 on device %s\n", dev->name);
	ext2_priv_data *priv = privd;

	inode_t ino;
	if(ext2_read_inode(&ino, 2, dev, priv) == 0)
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