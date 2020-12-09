#include "ext2FS.h"
#include "ext2.h"
#include "IODevice.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>


static int ext2FSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat);
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
};



static VFSFileSystem _fs = {.ops = &_ops};

VFSFileSystem* getExt2FS()
{
    return &_fs;
}

static int ext2FSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat)
{
    IODevice* dev = fs->data;
    assert(dev);
    printf("ext2FSStat req for '%s'\n", path);
    if (strcmp(path, "/") == 0)
    {
        printf("List EXT2 root \n");
        ext2_read_root_directory("hello", dev, NULL);
        return 0;   
    }

    const char* pp = path + 1;// skip '/'

    uint8_t ret = ext2_read_root_directory(pp, dev, NULL);

    return ret == 0? ENOENT : 0;

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