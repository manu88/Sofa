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
#include "cpio.h"
#include "Log.h"
#include <stdio.h>
#include <cpio/cpio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>

extern char _cpio_archive[];
extern char _cpio_archive_end[];

static char** _files = NULL;
static size_t numFiles = 0;

static int cpioFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int cpioFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);


static VFSFileSystemOps _ops =
{
    .Stat = cpioFSStat,
    .Open = cpioFSOpen,
};

static VFSFileSystem _cpioFS  = {.ops = &_ops};


static int cpioFSRead(ThreadBase* caller, File *file, void *buf, size_t numBytes);

static FileOps _fileOps = 
{
    .Read  = cpioFSRead,
    .asyncRead = 0
};


VFSFileSystem* getCpioFS()
{
    return &_cpioFS;
}

static int parseCpio()
{
    size_t  archLen = _cpio_archive_end - _cpio_archive;
    struct cpio_info info;
    int err = cpio_info(_cpio_archive, archLen, &info);
    if(err !=0)
    {
        return -ENOMEM;
    }

    _files = malloc(sizeof(char*)*info.file_count);
    if(!_files)
    {
        printf("malloc error\n");
        return -ENOMEM;
    }
    for(int i=0;i<info.file_count;i++)
    {
        _files[i] = malloc(info.max_path_sz);
    }
    numFiles = info.file_count;
    cpio_ls(_cpio_archive, archLen, _files, numFiles );
    return 0;
}

static int cpioFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat)
{
    if(!_files)
    {
        int ret = parseCpio();
        if(ret != 0)
        {
            return ret;
        }
    }
    if(numPathSegments == 0)
    {
        stat->type = FileType_Dir;
        return 0;
    }
    if(numPathSegments == 1 && strcmp(path[0], "/") == 0)
    {
        return 0;
    }
    if(numPathSegments > 1)
    {
        return -ENOENT;
    }
    for(int i=0;i<numFiles;i++)
    {
        if(strcmp(path[0], _files[i]) == 0)
        {
            return 0;
        }
    }
    return -ENOENT;
}

static int _ReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    size_t remainFilesToList = file->size - file->readPos;
    size_t numDirentPerBuff = numBytes / sizeof(struct dirent);
    size_t numOfDirents = numDirentPerBuff > remainFilesToList? remainFilesToList:numDirentPerBuff;
   
    struct dirent *dirp = buf;

    size_t nextOff = 0;
    size_t acc = 0;

    for(int i=0;i<numOfDirents;i++)
    {
        struct dirent *d = dirp + i;
        size_t fPos = i + file->readPos;
        assert(fPos <numFiles);
        snprintf(d->d_name, 256, "%s", _files[fPos]);
        acc += sizeof(struct dirent);
        d->d_off = acc;
        d->d_type = DT_DIR;
        d->d_reclen = sizeof(struct dirent);

        if(i >= numOfDirents)
        {
            break;
        }
    }
    file->readPos += numOfDirents;
    return acc;
}

static FileOps _rootFOP = {
    .Read = _ReadDir,
    .asyncRead = 0
    };

static int cpioFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    if(!_files)
    {
        int ret = parseCpio();
        if(ret != 0)
        {
            return ret;
        }
    }

    if(strcmp(path, "/") == 0)
    {

        file->ops = &_rootFOP;
        file->size = numFiles;
        return 0;
    }

    if(mode != O_RDONLY)
    {
        return -EROFS;
    }

    if(strcmp(path, "/") == 0)
    {
        KLOG_DEBUG("CPIO req for /\n");
    }
    const char* p = path + 1;// skip 1st '/'
    for(int i=0;i<numFiles;i++)
    {
        if(strcmp(p, _files[i]) == 0)
        {
            file->ops = &_fileOps;
            file->mode = O_RDONLY;
            size_t fSize;
            file->impl = cpio_get_file(_cpio_archive, _cpio_archive_end - _cpio_archive, p, &fSize);
            if(file->impl == NULL)
            {
                return -EFAULT;
            }
            file->size = fSize;

            return 0;
        }
    }

    return -ENOENT;
}

static int cpioFSRead(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    void* fData = file->impl;

    size_t effectiveSize = file->size - file->readPos;
    if(numBytes < effectiveSize)
    {
        effectiveSize = numBytes;
    } 

    memcpy(buf, fData + file->readPos, effectiveSize);

    return effectiveSize;
}
