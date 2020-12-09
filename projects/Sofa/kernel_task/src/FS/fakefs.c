#include "fakefs.h"
#include <stdio.h>
#include <errno.h>


static int fakeFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int fakeFSRead(File *file, void *buf, size_t numBytes);
static int fakeFSWrite(File *file, const void *buf, size_t numBytes);

static int fakeFSClose(File *file);

static VFSFileSystemOps _ops =
{
    .Stat = fakeFSStat,
    .Open = fakeFSOpen,
};

static FileOps _fileOps = 
{
    .Read = fakeFSRead,
    .Close = fakeFSClose,
    .Write = fakeFSWrite
};

static VFSFileSystem _fs = {.ops = &_ops};

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
        .content = "Hello this is the content of file2, wich is a little bit longuer in order to test the buffers.\nPlease Note That this sentence should begin at a new line.\n\tItem1\n\tItem2"

    },
};
#define NumFiles 2

VFSFileSystem* getFakeFS()
{
    return &_fs;
}

static int fakeFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat)
{
    if(numPathSegments == 0) // root
    {
        for(int i=0;i<NumFiles;i++)
        {
            printf("%s\n", files[i].name);
        }
        return 0;   

    }
    printf("fakeStat request %i\n", numPathSegments);
    for (int i=0;i<numPathSegments;i++)
    {
        printf("%s\n", path[i]);
    }
    return ENOENT;
}

static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    const char* p = path+1;

    for(int i=0;i<NumFiles;i++)
    {
        if(strcmp(p, files[i].name) == 0)
        {
            file->ops = &_fileOps;
            file->impl = &files[i];

            file->size = strlen(files[i].content);
            return 0;
        }
    }
    return ENOENT;
}

static int fakeFSClose(File *file)
{
    return 0;
}

static int fakeFSWrite(File *file, const void *buf, size_t numBytes)
{
    printf("FakeFS: request to write %zi '%s'\n", numBytes, buf);
    return 0;
}

static int fakeFSRead(File *file, void *buf, size_t numBytes)
{
    FakeFile* f = file->impl;
    if(!f)
    {
        return -1;
    }

    size_t effectiveSize = strlen(f->content) - file->readPos;
    if(numBytes < effectiveSize)
    {
        effectiveSize = numBytes;
    } 
    memcpy(buf, f->content + file->readPos, effectiveSize);

    return effectiveSize;
}