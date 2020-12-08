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

typedef struct 
{
    const char* name;
    const char* content;
} FakeFile;


static const FakeFile const files[] = 
{
    {
        .name = "file1",
        .content = "Hello this is the content of file1"
    },
    {
        .name = "file2",
        .content = "Hello this is the content of file2"

    },
};
#define NumFiles 2

VFSFileSystem* getFakeFS()
{
    return &_fs;
}

static int fakeFSStat(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat)
{
    if (strcmp(path, "/") == 0)
    {
        for(int i=0;i<NumFiles;i++)
        {
            printf("%s\n", files[i].name);
        }
        
    }
    return 0;
}


static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    const char* p = path+1;
    printf("FakeFS open request for '%s'\n", p);

    for(int i=0;i<NumFiles;i++)
    {
        if(strcmp(p, files[i].name) == 0)
        {
            file->ops = &_fileOps;
            file->impl = &files[i];
            return 0;
        }
    }

    return -1;
}

static int fakeFSClose(File *file)
{
    printf("FakeFS close request\n");
    return 0;
}



static int fakeFSRead(File *file, void *buf, size_t numBytes)
{
    printf("FakeFS read req for %zi bytes\n", numBytes);

    FakeFile* f = file->impl;
    if(!f)
    {
        return -1;
    }

    size_t effectiveSize = strlen(f->content);
    if(numBytes < effectiveSize)
    {
        effectiveSize = numBytes;
    } 
    memcpy(buf, f->content, effectiveSize);

    return effectiveSize;
}