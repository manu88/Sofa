#include "cpio.h"
#include "Log.h"
#include <stdio.h>
#include <cpio/cpio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

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


static int cpioFSRead(File *file, void *buf, size_t numBytes);

static FileOps _fileOps = 
{
    .Read  = cpioFSRead,
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
        return ENOMEM;
    }

    _files = malloc(sizeof(char*)*info.file_count);
    if(!_files)
    {
        printf("malloc error\n");
        return ENOMEM;
    }
    for(int i=0;i<info.file_count;i++)
    {
        _files[i] = malloc(info.max_path_sz);
    }
    numFiles = info.file_count;
    cpio_ls(_cpio_archive, archLen, _files, info.max_path_sz );
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

    for(int i=0;i<numFiles;i++)
    {
        printf("'%s'\n",_files[i]);
    }

    return 0;
}

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
    if(mode != O_RDONLY)
    {
        return EROFS;
    }

    const char* p = path + 1;// skip 1st '/'
    for(int i=0;i<numFiles;i++)
    {
        if(strcmp(p, _files[i]) == 0)
        {
            file->ops = &_fileOps;
            file->impl = _files[i];
            file->mode = O_RDONLY;
            size_t fSize;
            cpio_get_file(_cpio_archive, _cpio_archive_end - _cpio_archive, p, &fSize);
            file->size = fSize;
            return 0;
        }
    }

    return ENOENT;
}

static int cpioFSRead(File *file, void *buf, size_t numBytes)
{
    size_t fSize;
    void* fData = cpio_get_file(_cpio_archive, _cpio_archive_end - _cpio_archive, file->impl, &fSize);

    size_t effectiveSize = fSize - file->readPos;
    if(numBytes < effectiveSize)
    {
        effectiveSize = numBytes;
    } 
    memcpy(buf, fData + file->readPos, effectiveSize);
    return effectiveSize;
}
