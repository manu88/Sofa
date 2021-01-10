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
#include "ext2FS.h"
#include "ext2.h"
#include "IODevice.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>


static int ext2FSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int ext2FSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int ext2FSRead(File *file, void *buf, size_t numBytes);
static int ext2FSClose(File *file);

static VFSFileSystemOps _ops =
{
    .Stat = ext2FSStat,
    .Open = ext2FSOpen,
};

static FileOps _fileOps = 
{
    .Read =  ext2FSRead,
    .Close = ext2FSClose,
    .asyncRead = 0,
};



static VFSFileSystem _fs = {.ops = &_ops};

VFSFileSystem* getExt2FS()
{
    return &_fs;
}

static int ext2FSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat)
{
    IODevice* dev = fs->data;
    assert(dev);

    uint32_t block = 0; /* The block where this inode should be written */
	uint32_t ioff = 0; /* Offset into the block function to sizeof(inode_t) */
    ext2_get_inode_block(13, &block, &ioff, dev, NULL);
    printf("Block is at %zi ioff %zi\n", block, ioff);

    uint8_t bb[4096];
    printf("Read block:\n");
    uint8_t r =  ext2_read_block(bb, block, dev, NULL);
    printf("Read block: ret %i\n",r);
    ext2_dir* dir = (ext2_dir*)bb;
    dir += ioff*sizeof(inode_t);

    for (int i=0;i<ioff;i++)
    {
    }
    printf("Dir inode %i name len %i\n", dir->inode, dir->namelength);
    uint32_t rr = ext2_read_directory("", dir, dev, NULL);

    printf("did read directory %i\n", rr);
    if(numPathSegments == 0)
    {
        printf("List EXT2 root \n");
        ext2_read_root_directory("", dev, NULL);
        return 0;   
    }

    inode_t ino;
    if(ext2_find_file_inode(path[0], &ino, dev, NULL))
    {
        printf("Found inode\n");
    }
    return ENOENT;
    printf("ext2 stat request %i\n", numPathSegments);
    int remains = numPathSegments;
    int index = 0;
    while (remains--)
    {
        const char* seg = path[index];

        if(index == 0)
        {
            if(ext2_read_root_directory(seg, dev, NULL) == 0)
            {
                return ENOENT;
            }
        }
        

        printf("Process '%s'\n", seg);
        index++;
    }
    return ENOENT;
    
    for (int i=0;i<numPathSegments;i++)
    {
        //if(ext2_read_root_directory("", dev, NULL);
        printf("%s\n", path[i]);
    }
    return ENOENT;

}
static int ext2FSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    return -1;
}
static int ext2FSRead(File *file, void *buf, size_t numBytes)
{
    return -1;
}
static int ext2FSClose(File *file)
{
    return -1;
}