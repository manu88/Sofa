#include "fakefs.h"
#include <stdio.h>
#include <errno.h>


static int fakeFSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat);
static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int fakeFSRead(File *file, void *buf, size_t numBytes);
static int fakeFSClose(File *file);

static VFSFileSystemOps _ops =
{
    .Stat = fakeFSStat,
    .Open = fakeFSOpen
    
};


static FileOps _fileOps = 
{
    .Read = fakeFSRead,
    .Close = fakeFSClose
};

VFSFileSystem _fs = {.ops = &_ops};


static const char* const files[] = { "file1", "file2"};
#define NumFiles (sizeof (files) / sizeof (const char *))

VFSFileSystem* getFakeFS()
{
    return &_fs;
}

static int fakeFSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat)
{
    printf("FakeFS stat request for '%s'\n", path);
    if (strcmp(path, "/") == 0)
    {
        for(int i=0;i<NumFiles;i++)
        {
            printf("%s\n", files[i]);
        }
        
    }
    return 0;
}


static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    printf("FakeFS open request for '%s'\n", path);
    file->ops = &_fileOps;
    return 0;
}

static int fakeFSClose(File *file)
{
    printf("FakeFS close request\n");
    return 0;
}



static int fakeFSRead(File *file, void *buf, size_t numBytes)
{
    return -1;
}