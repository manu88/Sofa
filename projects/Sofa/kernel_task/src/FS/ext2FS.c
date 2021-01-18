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
//#include "ext2.h"
#include "Ext2.h"
#include "IODevice.h"
#include "Log.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

static inode_t _rootInode;
static int _reReadRootInode = 1;

static int ext2FSStat(VFSFileSystem* fs, const char*path, VFS_File_Stat* stat);

static int ext2FSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int ext2FSRead(ThreadBase* caller,File *file, void *buf, size_t numBytes);
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


static int ext2FSStat(VFSFileSystem* fs, const char*path, VFS_File_Stat* stat)
{
    if(strcmp(path, "/") == 0)
    {
        stat->type = FileType_Dir;
        return 0;
    }
    IODevice* dev = fs->data;
    assert(dev);

    const char* fPath = path+1;

    if(_reReadRootInode)
    {
        if(!Ext2ReadInode(&_rootInode, 2, dev))
        {
            return -EIO;
        }
        _reReadRootInode = 0;
    }

    
    char tmpName[256] = "";
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = _rootInode.dbp[i];
		if(b == 0)
        {
            break;
        }

        uint8_t* root_buf = Ext2ReadBlockCached(b, dev);
        if(!root_buf)
        {
            return -EIO;
        }

        ext2_dir* dir = (ext2_dir*) root_buf;

        while(dir->inode != 0) 
        {
            memcpy(tmpName, &dir->reserved+1, dir->namelength);
            tmpName[dir->namelength] = 0;
            if(strcmp(tmpName, fPath) == 0)
            {
                inode_t ino;

                int retRead = Ext2ReadInode(&ino, dir->inode, dev); 
 
                if(retRead)
                {

                    if((ino.type & 0xF000) == INODE_TYPE_DIRECTORY)
                    {
                        stat->type = FileType_Dir;
                    }
                    else if((ino.type & 0xF000) == INODE_TYPE_FILE)
                    {
                        stat->type = FileType_Regular;
                    }
                }                
                return retRead? 0: -EIO;
            }
            dir = (ext2_dir *)((uint32_t)dir + dir->size);
            ptrdiff_t dif = (char*) dir - (char*) root_buf;
            if( dif >= getExtPriv()->blocksize)
            {
                return -EIO;
            }
        }
    }

    return -ENOENT;

}

static int ext2FSReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{

    if(file->size)
    {
        return 0;
    }
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

    //uint8_t* root_buf = (uint8_t *)malloc(getExtPriv()->blocksize);
    //assert(root_buf);
    char tmpName[256] = "";
    int ii=0;
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = ino->dbp[i];
		if(b == 0)
        {
            break;
        }
        uint8_t*root_buf=  Ext2ReadBlockCached(b, dev);
        if(!root_buf)
        {
            return -EIO;
        }
        ext2_dir* dir = (ext2_dir*) root_buf;

        while(dir->inode != 0) 
        {
            if(dir->namelength < 255)
            {
                memcpy(tmpName, &dir->reserved+1, dir->namelength);
                tmpName[dir->namelength] = 0;

                if(strlen(tmpName) && 
                   strcmp(tmpName, ".") != 0 &&
                   strcmp(tmpName, "..") != 0 )
                {
                    struct dirent *d = dirp + ii;
                    snprintf(d->d_name, 256, "%s", tmpName);

                    if(strlen(tmpName) == dir->namelength)
                    {
                        acc += sizeof(struct dirent);
                        d->d_off = acc;
                        d->d_type = DT_DIR;
                        d->d_reclen = sizeof(struct dirent);

                        file->size +=1;
                        ii+=1;
                        if(ii >numDirentPerBuff)
                        {
                            return acc;
                        }
                    }
                }
            }

            dir = (ext2_dir *)((uint32_t)dir + dir->size);
            ptrdiff_t dif = (char*) dir - (char*) root_buf;
            if( dif >= getExtPriv()->blocksize)
            {
                return acc;
            }
        }
    }

    return acc;
}

static int ext2FSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    IODevice* dev = fs->data;
    assert(dev);

    if(_reReadRootInode)
    {
        if(!Ext2ReadInode(&_rootInode, 2, dev))
        {
            return -EIO;
        }
        _reReadRootInode = 0;
    }
    
    if(strcmp(path, "/") == 0)
    {
        if((_rootInode.type & 0xF000) != INODE_TYPE_DIRECTORY)
        {
            return -ENOTDIR;
        }

        file->impl = dev;
        file->inode = &_rootInode;
        file->ops = &_dirOP;
        return 0;
    }
    const char *fPath = path+1;
    
    char tmpName[256] = "";
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = _rootInode.dbp[i];
		if(b == 0)
        {
            break;
        }
		uint8_t* root_buf = Ext2ReadBlockCached(b, dev);
        if(!root_buf)
        {
            return -EIO;
        }

        ext2_dir* dir = (ext2_dir*) root_buf;
        
        while(dir->inode != 0) 
        {
            memcpy(tmpName, &dir->reserved+1, dir->namelength);
            tmpName[dir->namelength] = 0;
            if(strcmp(tmpName, fPath) == 0)
            {
                file->inode = malloc(sizeof(inode_t));
                file->impl = dev;
                file->ops = &_fileOps;

                int ret = Ext2ReadInode(file->inode, dir->inode, dev);
                if(ret)
                {
                    file->size = ((inode_t*) file->inode)->size;
                }
                return ret? 0:-EIO;
            }
            dir = (ext2_dir *)((uint32_t)dir + dir->size);
            ptrdiff_t dif = (char*) dir - (char*) root_buf;
            if( dif >= getExtPriv()->blocksize)
            {
                return -EIO;
            }
        }
    }

    return -ENOENT;
}


static int ext2FSRead(ThreadBase* caller,File *file, void *buf, size_t numBytes)
{

    IODevice* dev = file->impl;
    inode_t* inode = file->inode;
    assert(dev);
    assert(inode);



    if(file->readPos >= file->size)
    {
        return 0;
    }
    size_t indexOfBlockToRead = (size_t)file->readPos / getExtPriv()->blocksize;
    int i = indexOfBlockToRead;

    if(i<12)
    {
        uint32_t b = inode->dbp[i];
        if(b==0)
        {
            KLOG_DEBUG("EOF For read\n");
            // EOF
            return 0;
        }
        if(b > getExtPriv()->sb.blocks) 
        {
            KLOG_DEBUG("block %d outside range (max: %d)!\n",  b, getExtPriv()->sb.blocks);
        }
        uint8_t* tempBuf = Ext2ReadBlockCached(b, dev);
        if(!tempBuf)
        {
            return -EIO;
        }
        size_t sizeToCopy = file->size - file->readPos;
        if(sizeToCopy > numBytes)
        {
            sizeToCopy = numBytes;
        }
        size_t chunkToCopy = file->readPos % getExtPriv()->blocksize;
        memcpy(buf, tempBuf + chunkToCopy, sizeToCopy);
        file->readPos += sizeToCopy;
        return sizeToCopy;
    }
    else if(i >= 12)
    {
        if(inode->doubly_block)
        {
            KLOG_DEBUG("Doubly block, to implement :)\n");
            assert(0);
        }
        if(inode->triply_block)
        {
            KLOG_DEBUG("Triply block, to implement :)\n");
            assert(0);
        }
        if(inode->singly_block)
        {
            i -= 12;

            uint32_t *block = (uint32_t *) Ext2ReadBlockCached(inode->singly_block, dev);
            if(!block)
            {
                return -EIO;
            }
            uint32_t maxblocks = ((getExtPriv()->blocksize) / (sizeof(uint32_t)));

            if(block[i] == 0)
            {
                KLOG_DEBUG("block[%i] is 0 ?\n", i);
            }
            else
            {
                uint8_t* data = Ext2ReadBlockCached(block[i], dev);

                if(!data)
                {
                    return -EIO;
                }
                size_t sizeToCopy = file->size - file->readPos;
                if(sizeToCopy > numBytes)
                {
                    sizeToCopy = numBytes;
                }
                size_t chunkToCopy = file->readPos % getExtPriv()->blocksize;
                memcpy(buf, data + chunkToCopy, sizeToCopy);
                file->readPos += sizeToCopy;
                return sizeToCopy;
            }
        }
    }
    
    return 0;
}
static int ext2FSClose(File *file)
{
    //free(file->inode);
    return -1;
}