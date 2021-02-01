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

#define ROOT_INODE_ID 2


typedef struct
{
    inode_t ino;
    
    uint32_t inoId;
    UT_hash_handle hh;
} Inode;

static Inode* _inodes;

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


inode_t * ReadInode(uint32_t inode, IODevice *dev)
{
    Inode * ino = NULL;
    HASH_FIND_INT(_inodes, &inode, ino);
    if(ino)
    {
        return &ino->ino;
    }

    ino = malloc(sizeof(Inode));
    
    if(Ext2ReadInode(&ino->ino, inode, dev))
    {
        ino->inoId = inode;
        HASH_ADD_INT(_inodes, inoId, ino);
        return &ino->ino;
    }
    free(ino);
    return NULL;
}

static inode_t * getInodeNamed(IODevice* dev, inode_t* inode, const char* path)
{
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(b == 0)
        {
            break;
        }

        uint8_t* blockData = Ext2ReadBlockCached(b, dev);
        if(!blockData)
        {
            return NULL;
        }

        ext2_dir* dir = (ext2_dir*) blockData;

        char tmpName[256] = "";
        while(dir->inode != 0) 
        {
            memcpy(tmpName, &dir->reserved+1, dir->namelength);
            tmpName[dir->namelength] = 0;

            if(strcmp(tmpName, path) == 0)
            {
                inode_t *ino = ReadInode(dir->inode, dev);

                return ino;
            }
            dir = (ext2_dir *)((uint32_t)dir + dir->size);
            ptrdiff_t dif = (char*) dir - (char*) blockData;
            if( dif >= getExtPriv()->blocksize)
            {
                return NULL;
            }
        }
    }

    return NULL;
}

static inode_t _rootInode;
static inode_t * _GetInodeForPath(VFSFileSystem* fs, const char* path)
{
    IODevice* dev = fs->data;
    assert(dev);

    if(_reReadRootInode)
    {
        if(!Ext2ReadInode(&_rootInode, ROOT_INODE_ID, dev))
        {
            return NULL;
        }
        _reReadRootInode = 0;
    }

    if(strcmp(path, "/") == 0)
    {
        return &_rootInode;
    }

    inode_t* currentInode = &_rootInode;

    size_t numSegs = 0;
    char** paths = VFSSplitPath(path, &numSegs);
    if(paths == NULL)
    {
        return NULL;
    }
    for(size_t i = 0;i<numSegs;++i)
    {
        inode_t* nextInode = getInodeNamed(dev, currentInode, paths[i]);
        if(nextInode)
        {
            // not last and not a dir!
            if(i < numSegs-1 && (nextInode->type & 0xF000) != INODE_TYPE_DIRECTORY)
            {
                currentInode = NULL;
                goto cleanup;
            }
            currentInode = nextInode;
        }
        else
        {
            currentInode = NULL;
            goto cleanup;
        }   
    }

cleanup:
    for(size_t i = 0;i<numSegs;++i)
    {
        free(paths[i]);
    }
    free(paths);
    return currentInode;
}


static int ext2FSStat(VFSFileSystem* fs, const char*path, VFS_File_Stat* stat)
{
    inode_t* inode =  _GetInodeForPath(fs, path);
    if(inode)
    {
        if((inode->type & 0xF000) == INODE_TYPE_DIRECTORY)
        {
            stat->type = FileType_Dir;
        }
        else if((inode->type & 0xF000) == INODE_TYPE_FILE)
        {
            stat->type = FileType_Regular;
        }
        else 
        {
            KLOG_DEBUG("Other file type %X\n", (inode->type & 0xF000));
            assert(0);
        }
        return 0;
    }
    return -ENOENT;
}

static int ext2FSReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    assert(file);
    assert(file->inode);
    inode_t* ino = file->inode;    
    if(file->size)
    {
        return 0;
    }
    size_t numDirentPerBuff = numBytes / sizeof(struct dirent);
    size_t numOfDirents = numDirentPerBuff;
   
    struct dirent *dirp = buf;


    IODevice* dev = file->impl;
    assert(dev);
    if(ino == NULL)
    {
        return -EINVAL;
    }

    size_t nextOff = 0;
    size_t acc = 0;

    char tmpName[256] = "";
    int ii=0;
    for(int i = 0;i < 12; i++)
	{
		uint32_t b = ino->dbp[i];
		if(b == 0)
        {
            break;
        }
        uint8_t*dir_buf=  Ext2ReadBlockCached(b, dev);
        if(!dir_buf)
        {
            return -EIO;
        }
        ext2_dir* dir = (ext2_dir*) dir_buf;

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
            ptrdiff_t dif = (char*) dir - (char*) dir_buf;
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
    assert(fs->data);
    IODevice* dev = fs->data;    
    inode_t* inode =  _GetInodeForPath(fs, path);
    if(!inode)
    {
        return -ENOENT;
    }

    file->impl = dev;
    file->inode = inode;

    if((inode->type & 0xF000) == INODE_TYPE_DIRECTORY)
    {
        file->ops = &_dirOP;
        return 0;
    }
    else if((inode->type & 0xF000) == INODE_TYPE_FILE)
    {
        file->ops = &_fileOps;       
        file->size = inode->size;                                        
        return 0;
    }
    assert(0);
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