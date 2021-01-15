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
#include "devFS.h"
#include "Log.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include "Log.h"
#include "../utils.h"
#include "IODevice.h"

static int devFSStat(VFSFileSystem* fs, const char*path, VFS_File_Stat* stat);
static int devFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static VFSFileSystemOps _ops =
{
    .Stat = devFSStat,
    .Open = devFSOpen,
};


static VFSFileSystem _fs = {.ops = &_ops};

static DevFile* _devFiles = NULL; 

VFSFileSystem* getDevFS()
{
    return &_fs;
}

DevFile* DevFSGetFileForDevice( const IODevice* dev)
{
    DevFile* f = NULL;
    DevFile* tmp = NULL;
    HASH_ITER(hh, _devFiles, f, tmp)
    {
        if(f->device == dev)
        {
            return f;
        }
    } 
    return NULL;
}

int DevFSAddDev(DevFile* file)
{
    KLOG_DEBUG("DevFSAddDev add device %s\n", file->name);
    HASH_ADD_STR(_devFiles, name, file);
    return 0;
}

static int devFSStat(VFSFileSystem* fs, const char*path, VFS_File_Stat* stat)
{
    if(strcmp(path, "/") == 0)
    {
        stat->type = FileType_Dir;
        return 0;
    }

    const char* p = path+1;
    DevFile* f = NULL;
    HASH_FIND_STR(_devFiles, p, f);

    if(!f)
    {
        return -ENOENT;
    }

    if(f->device->type == IODevice_BlockDev)
    {
        stat->type = FileType_Block;
    }
    else if(f->device->type == IODevice_CharDev)
    {
        stat->type = FileType_Char;
    }
    
    return 0;
}

static int _ReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    size_t remainFilesToList = file->size - file->readPos;
    size_t numDirentPerBuff = numBytes / sizeof(struct dirent);
    size_t numOfDirents = numDirentPerBuff > remainFilesToList? remainFilesToList:numDirentPerBuff;
   
    if(remainFilesToList == 0)
    {
        return 0;
    }
    struct dirent *dirp = buf;

    size_t nextOff = 0;
    size_t acc = 0;

    int i =0;

    DevFile* f = NULL;
    DevFile* tmp = NULL;
    HASH_ITER(hh, _devFiles, f, tmp)
    {
        struct dirent *d = dirp + i;
        size_t fPos = i + file->readPos;
        
        snprintf(d->d_name, 256, "%s", f->name);
        acc += sizeof(struct dirent);
        d->d_off = acc;
        d->d_type = DT_CHR;
        d->d_reclen = sizeof(struct dirent);
        i++;
        if(i >= numOfDirents)
        {
            break;
        }

    }
    file->readPos += numOfDirents;
    return acc;
}

static FileOps _rootFOP = 
{
    .Read = _ReadDir,
    .asyncRead = 0
};

static int devFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    if(strcmp(path, "/") == 0)
    {

        file->ops = &_rootFOP;
        file->size = HASH_COUNT(_devFiles);
        return 0;
    }
    const char* p = path+1;
    
    DevFile* f = NULL;
    HASH_FIND_STR(_devFiles, p, f);

    if(!f)
    {
        return -ENOENT;
    }
    file->ops = f->ops;
    file->impl = f->device;
    file->size = -1;
    file->mode = O_RDWR;
    return 0;

}
