#include "fakefs.h"
#include "Log.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>


static int fakeFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int fakeFSRead(File *file, void *buf, size_t numBytes);
static int fakeFSClose(File *file);
static int fakeFSWrite(File *file, const void *buf, size_t numBytes);

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

    int mode;
} FakeFile;


static const FakeFile const files[] = 
{
    {
        .name = "file1",
        .content = "Hello this is the content of file1",
        .mode = O_RDONLY
    },
    {
        .name = "file2",
        .content = "Hello this is the content of file2, wich is a little bit longuer in order to test the buffers.\nPlease Note That this sentence should begin at a new line.\n\tItem1\n\tItem2",
        .mode = O_RDONLY
    },
    {
        .name = "file3",
        .content = "Hello this is the content of file1",
        .mode = O_RDONLY
    },
    {
        .name = "file4",
        .content = "Hello this is the content of file2, wich is a little bit longuer in order to test the buffers.\nPlease Note That this sentence should begin at a new line.\n\tItem1\n\tItem2",
        .mode = O_RDONLY
    },
    {
        .name = "file5",
        .content = "Hello this is the content of file1",
        .mode = O_RDONLY
    },
    {
        .name = "file6",
        .content = "Hello this is the content of file2, wich is a little bit longuer in order to test the buffers.\nPlease Note That this sentence should begin at a new line.\n\tItem1\n\tItem2",
        .mode = O_RDONLY
    },
    {
        .name = "file7",
        .content = "Hello this is the content of file1",
        .mode = O_RDONLY
    },
    {
        .name = "file8",
        .content = "Hello this is the content of file2, wich is a little bit longuer in order to test the buffers.\nPlease Note That this sentence should begin at a new line.\n\tItem1\n\tItem2",
        .mode = O_RDONLY
    },
    {
        .name = "cons",
        .mode = O_WRONLY
    },
};
#define NumFiles 9

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
            printf("%s %i\n", files[i].name, files[i].mode);
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

static int _ReadDir(File *file, void *buf, size_t numBytes)
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
        assert(fPos <NumFiles);
        snprintf(d->d_name, 256, "%s", files[fPos].name);
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

static FileOps _rootFOP = {.Read = _ReadDir};

static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    if(strcmp(path, "/") == 0)
    {

        file->ops = &_rootFOP;
        file->size = NumFiles;
        return 0;
    }
    const char* p = path+1;

    for(int i=0;i<NumFiles;i++)
    {
        if(strcmp(p, files[i].name) == 0)
        {
            file->ops = &_fileOps;
            file->impl = &files[i];

            if(files[i].content)
            {
                file->size = strlen(files[i].content);
            }
            else
            {
                file->size = 0;
            }
            if(mode == files[i].mode)
            {
                file->mode = files[i].mode;
                return 0;
            }
            return EACCES;
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
    printf("%s",(const char*) buf);
    fflush(stdout);
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

    file->readPos += effectiveSize;

    return effectiveSize;
}