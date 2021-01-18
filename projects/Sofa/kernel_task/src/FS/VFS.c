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
#include "VFS.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include "Log.h"
#define MAX_PREFIX_LEN 256

static VFSMountPoint* _mountPoints = NULL;

/*
 * Unpack a path into prefix and suffix.
 * The prefix determines which mounted filesystem
 * the path resides in.  The suffix is the path within
 * the filesystem.
 * Params:
 *   path - the complete path
 *   prefix - buffer where prefix will be stored
 *   pSuffix - stores the pointer to the suffix part of path
 * Returns: true if path is valid, false if not
 */
static uint8_t Unpack_Path(const char *path, char *prefix, const char **pSuffix)
{
    char *slash;
    size_t pfxLen;

    /* Path must start with '/' */
    if (*path != '/')
	    return 0;
    ++path;

    /* Look for the initial slash. */
    slash = strchr(path, '/');
    if (slash == 0)
    {
        /*
        * Special case: path of the form "/prefix".
        * It resolves to the root directory of
        * the filesystem mounted on the prefix.
        */
        pfxLen = strlen(path);
        if (pfxLen == 0 || pfxLen > MAX_PREFIX_LEN)
            return 0;
        strcpy(prefix, path);
        *pSuffix = "/";
    } 
    else
    {
        /*
        * Determine length of file prefix.
        * It needs to be non-zero, but less than MAX_PREFIX_LEN.
        */
        pfxLen = slash - path;
        if (pfxLen == 0 || pfxLen > MAX_PREFIX_LEN)
            return 0;

        /* Format the path prefix as a string */
        memcpy(prefix, path, pfxLen);
        prefix[pfxLen] = '\0';

        /*
        * Set pointer to "suffix", i.e., the rest of the path
        * after the prefix
        */
        *pSuffix = slash;
    }

    assert(**pSuffix == '/');

    return 1;
}

VFSMountPoint* _GetMountPoint(const char* mountPath)
{
    VFSMountPoint* mntPt = NULL;
    HASH_FIND_STR(_mountPoints, mountPath, mntPt);
    return mntPt;
}

static int doMountFS(VFSMountPoint* mntpt, VFSFileSystem* fs, const char* path)
{
    mntpt->mountPath = strdup(path);
    mntpt->fs = fs;
    HASH_ADD_STR(_mountPoints, mountPath, mntpt);

    return 0;

}

VFSMountPoint* VFSMount(VFSFileSystem* fs, const char* mntPoint, int*err)
{
    assert(fs);
    assert(mntPoint);

    if(fs->ops->Mount)
    {
        if(fs->ops->Mount(fs, NULL) != 0)
        {
            return NULL;
        }
    }

    VFSMountPoint *pt = malloc(sizeof(VFSMountPoint));
    if(pt == NULL)
    {
        *err = ENOMEM;
        return NULL;
    }

    /* Skip leading slash character(s) */
    while (*mntPoint == '/')
    {
	    ++mntPoint;
    }

    if (strlen(mntPoint) > MAX_PREFIX_LEN)
    {
        KLOG_INFO("[VFSMount] name too long '%s'\n", mntPoint);
	    *err = ENAMETOOLONG;
        free(pt);
        return NULL;
    }

    if(_GetMountPoint(mntPoint))
    {
        KLOG_INFO("[VFSMount] mount point already exists '%s'\n", mntPoint);

        *err = EEXIST;
        free(pt);
        return NULL;
    }
    *err = doMountFS(pt, fs, mntPoint);
    if(*err != 0)
    {
        free(pt);
    }

    return pt;
}

int VFSInit()
{
    return 0;
}


char** VFSSplitPath(const char* _path, size_t* numSegs)
{
    char delim[] = "/";

    char* path = strdup(_path);
    char *ptr = strtok(path, delim);
    size_t num = 0;

    char** segments = NULL;
    while(ptr != NULL)
	{
//		printf("%i '%s'\n", num, ptr);

        segments = realloc(segments, (num+1)*sizeof(char*));
        assert(segments);
        segments[num] = strdup(ptr);
        num++;
		ptr = strtok(NULL, delim);        
	}
    *numSegs = num;
    free(path);
    return segments;
}


int VFSStat(const char *path, VFS_File_Stat *stat)
{
    if(strcmp(path, "/") == 0)
    {
        stat->type = FileType_Dir;
        return 0;
    }
    char prefix[MAX_PREFIX_LEN + 1];
    const char *suffix;

    if (!Unpack_Path(path, prefix, &suffix))
    {
	    return -ENOENT;
    }

    VFSMountPoint* mnt = _GetMountPoint(prefix);
    if(mnt == NULL)
    {
        return -ENOENT;
    }
    assert(mnt->fs);

    if(mnt->fs->ops->Stat)
    {
        return mnt->fs->ops->Stat(mnt->fs, suffix, stat);
    }
    KLOG_DEBUG("VFS: unimplemented Stat for %s\n", mnt->mountPath);
    return -ENODEV;
}

static int VFSReadDir(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    if(file->readPos == file->size)
    {
        return -1; // EOF
    }

    size_t remainFilesToList = file->size - file->readPos;
    size_t numDirentPerBuff = numBytes / sizeof(struct dirent);
    size_t numOfDirents = numDirentPerBuff > remainFilesToList? remainFilesToList:numDirentPerBuff;
   
    struct dirent *dirp = buf;

    size_t nextOff = 0;
    size_t acc = 0;

    VFSMountPoint* fs = NULL;
    VFSMountPoint* tmp = NULL;
    size_t i = 0;
    HASH_ITER(hh, _mountPoints,fs, tmp)
    {
        struct dirent *d = dirp + i;
        snprintf(d->d_name, 256, "%s", fs->mountPath);
        acc += sizeof(struct dirent);
        d->d_off = acc;
        d->d_type = DT_DIR;
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
    .Read = VFSReadDir,
    .asyncRead = 0
};

int VFSOpen(const char* path, int mode, File* file)
{
    memset(file, 0, sizeof(File));
    if(strcmp(path, "/") == 0)
    {
        file->ops = &_rootFOP;
        file->size = HASH_COUNT(_mountPoints);
        return 0;
    }
    char prefix[MAX_PREFIX_LEN + 1];
    const char *suffix;

    if (!Unpack_Path(path, prefix, &suffix))
    {
	    return -ENOENT;
    }

    VFSMountPoint* mnt = _GetMountPoint(prefix);
    if(mnt == NULL)
    {
        return -ENOENT;
    }
    assert(mnt->fs);
    if(mnt->fs->ops && mnt->fs->ops->Open)
    {
        return mnt->fs->ops->Open(mnt->fs, suffix, mode, file);
    }
    return -EACCES;
}

int VFSClose(File* file)
{
    if(file->ops == NULL || file->ops->Close == NULL)
    {
        return 0;
    }
    return file->ops->Close(file);
}

int VFSSeek(File* file, size_t pos)
{
    assert(0);
    if(file->ops->Seek == NULL)
    {
        size_t effectiveOffset = pos;
        if(effectiveOffset > file->size)
        {
            effectiveOffset = file->size;
        }
        file->readPos = effectiveOffset;
        return 0;
    }
    return file->ops->Seek(file, pos);
}

ssize_t VFSWrite(File* file, const char* buf, size_t sizeToWrite)
{
    if(!file->ops ||!file->ops->Write)
    {
        return -EACCES;
    }
    if(file->mode == O_WRONLY || file->mode == O_RDWR)
    {
        return file->ops->Write(file, buf, sizeToWrite);
    }
    return -EACCES;
    
}

ssize_t VFSRead(ThreadBase* caller, File* file, char* buf, size_t sizeToRead, int *async_later)
{
    if(!file->ops ||!file->ops->Read)
    {
        return -EACCES;
    }
    assert(file->readPos <= file->size);
    if(async_later)
    {
        *async_later = file->ops->asyncRead;
    }

    return file->ops->Read(caller, file, buf, sizeToRead);
}