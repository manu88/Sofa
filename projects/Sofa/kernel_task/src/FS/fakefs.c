#include "fakefs.h"
#include <stdio.h>

static int fakeFSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat);


static VFSFileSystemOps _ops ={.Stat = fakeFSStat};
VFSFileSystem _fs = {.ops = &_ops};

VFSFileSystem* getFakeFS()
{
    return &_fs;
}

static int fakeFSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat)
{
    printf("FakeFS stat request for '%s'\n", path);
    return 0;
}




