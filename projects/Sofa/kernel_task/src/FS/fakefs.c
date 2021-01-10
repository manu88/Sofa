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
#include "fakefs.h"
#include "Log.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include "Serial.h"
#include "../utils.h"

static int fakeFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int fakeFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int fakeFSRead(ThreadBase* caller, File *file, void *buf, size_t numBytes);
static int consRead(ThreadBase* caller, File *file, void *buf, size_t numBytes);
static int fakeFSWrite(File *file, const void *buf, size_t numBytes);


static VFSFileSystemOps _ops =
{
    .Stat = fakeFSStat,
    .Open = fakeFSOpen,
};

static FileOps _fileOps = 
{
    .Read = fakeFSRead,
    .Write = fakeFSWrite,
    .asyncRead = 0
};

static FileOps _consoleOps = 
{
    .asyncRead = 1,
    .Read = consRead,
};

static VFSFileSystem _fs = {.ops = &_ops};

typedef struct 
{
    const char* name;
    const char* content;

    int mode;
    FileOps *ops;
} FakeFile;


static const FakeFile const files[] = 
{
    {
        .name = "file1",
        .content = "Hello this is the content of file1\n",
        .mode = O_RDONLY,
        .ops = &_fileOps
    },
    {
        .name = "file2",
        .content = "Hello this is the content of file2, which is a little bit longuer in order to test the buffers.\nPlease Note That this sentence should begin at a new line.\n\tItem1\n\tItem2\n",
        .mode = O_RDONLY,
        .ops = &_fileOps
    },
    {
        .name = "script",
        .content = "ls /\nps\n",
        .mode = O_RDONLY,
        .ops = &_fileOps
    },
    {
        .name = "cons",
        .mode = O_WRONLY,
        .ops = &_fileOps
    },
    {
        .name = "consin",
        .mode = O_RDONLY,
        .ops = &_consoleOps
    },
};
#define NumFiles 5

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

static FileOps _rootFOP = 
{
    .Read = _ReadDir,
    .asyncRead = 0
};

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
            file->ops = files[i].ops;
            file->impl = &files[i];


            if(files[i].content)
            {
                file->size = strlen(files[i].content);
            }
            else
            {
                file->size = -1;
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


static void onBytesAvailable(size_t size, char until, void* ptr, void* buf)
{
    ThreadBase* caller = (ThreadBase*) ptr;
    assert(caller);

    size_t bytes = SerialCopyAvailableChar(buf, size);
    ((char*)buf)[bytes] = 0;

    seL4_MessageInfo_t msg = seL4_MessageInfo_new(0, 0, 0, 3);
    seL4_SetMR(1, 0);
    seL4_SetMR(2, bytes);            
    
    seL4_Send(caller->replyCap, msg);
    cnode_delete(&getKernelTaskContext()->vka, caller->replyCap);
    caller->replyCap = 0;
    caller->state = ThreadState_Running;

}


static int consRead(ThreadBase* caller, File *file, void *buf, size_t numBytes)
{
    KernelTaskContext* env = getKernelTaskContext();

    assert(caller->replyCap == 0);

    seL4_Word slot = get_free_slot(&env->vka);
    int error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        KLOG_TRACE("Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        return -ENOMEM;
    }

    caller->replyCap = slot;
    caller->currentSyscallID = 0;
    SerialRegisterWaiter(onBytesAvailable, numBytes, '\n', caller, buf);
    //SerialRegisterController(onControlChar, caller);


    return -1;
}

static int fakeFSWrite(File *file, const void *buf, size_t numBytes)
{
    printf("%s",(const char*) buf);
    fflush(stdout);
    return 0;
}

static int fakeFSRead(ThreadBase* caller, File *file, void *buf, size_t numBytes)
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