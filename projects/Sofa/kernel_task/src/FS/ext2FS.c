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
#include "Log.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>


static int ext2FSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int ext2FSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int ext2FSRead(File *file, void *buf, size_t numBytes);
static int ext2FSClose(File *file);

static int ext2FSReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes);

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

static FileOps _dirOP = 
{
    .Read = ext2FSReadDir,
    .asyncRead = 0
};

static VFSFileSystem _fs = {.ops = &_ops};

VFSFileSystem* getExt2FS()
{
    return &_fs;
}

static int ext2FSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat)
{
    assert(0);
    KLOG_DEBUG("ext2FSStat\n");
    IODevice* dev = fs->data;
    
}

static int ext2FSReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    if(file->size)
    {
        return 0;
    }

    KLOG_DEBUG("ext2FSReadDir request\n");
    size_t numDirentPerBuff = numBytes / sizeof(struct dirent);
    size_t numOfDirents = numDirentPerBuff;
   
    struct dirent *dirp = buf;

    inode_t* ino = file->inode;
    IODevice* dev = file->impl;
    if(ino == NULL)
    {
        return -EINVAL;
    }

    size_t nextOff = 0;
    size_t acc = 0;


    uint8_t* root_buf = (uint8_t *)malloc(getExtPriv()->blocksize);
    assert(root_buf);

    char tmpName[256] = "";
    struct dirent *d = NULL;
    int ii=0;
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = ino->dbp[i];
		if(b == 0)
        {
            break;
        }
		uint8_t ret = ext2_read_block(root_buf, b, dev, getExtPriv());
        assert(ret == 1);
        ext2_dir* dir = (ext2_dir*) root_buf;
        
        while(dir->inode != 0) 
        {
            memcpy(tmpName, &dir->reserved+1, dir->namelength);
            tmpName[dir->namelength] = 0;
            if(strcmp(tmpName, ".") != 0 && strcmp(tmpName, "..") != 0 )
            {
                d = dirp + ii;
                snprintf(d->d_name, 256, "%s", tmpName);
                acc += sizeof(struct dirent);
                d->d_off = acc;
                d->d_type = DT_DIR;
                d->d_reclen = sizeof(struct dirent);

                file->size +=1;
                ii+=1;
            }

            dir = (ext2_dir *)((uint32_t)dir + dir->size);
        }
    }
    free(root_buf);

    return acc;
}
static int ext2FSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    KLOG_DEBUG("ext2FS Open for '%s'\n", path);

    IODevice* dev = fs->data;
    assert(dev);
    if(strcmp(path, "/") == 0)
    {
        inode_t *ino = malloc(sizeof(inode_t));
        uint8_t ret = ext2_read_inode(ino, 2, dev, getExtPriv());
        if(ret != 1) // err
        {
            free(ino);
            return -ENOENT;
        }
        if((ino->type & 0xF000) != INODE_TYPE_DIRECTORY)
        {
            free(ino);
            return -ENOTDIR;
        }
        file->impl = dev;
        file->inode = ino;
//        file->inodeNum = 2;
        file->ops = &_dirOP;
    }
    return 0;
#if 0
    uint8_t* root_buf = (uint8_t *)malloc(getExtPriv()->blocksize);
    assert(root_buf);
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = ino.dbp[i];
		if(b == 0)
        {
            break;
        }

		ret = ext2_read_block(root_buf, b, dev, getExtPriv());
        assert(ret == 1);
        ext2_dir* dir = (ext2_dir*) root_buf; 
        while(dir->inode != 0) 
        {
            char *name = (char *)malloc(dir->namelength + 1);
            assert(name);
            memcpy(name, &dir->reserved+1, dir->namelength);
            name[dir->namelength] = 0;
            KLOG_DEBUG("Got '%s'\n", name);
            free(name);
            dir = (ext2_dir *)((uint32_t)dir + dir->size);
        }
    }
    free(root_buf);
    KLOG_DEBUG("r=%u\n", ret);
    return -ENOENT;
#endif
}


static int ext2FSRead(File *file, void *buf, size_t numBytes)
{
    return -1;
}
static int ext2FSClose(File *file)
{
    return -1;
}