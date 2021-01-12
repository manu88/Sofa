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
#include "devFS.h"
#include "Log.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include "Serial.h"
#include "Log.h"
#include "../utils.h"


static int devFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
static int devFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file);

static int consRead(ThreadBase* caller, File *file, void *buf, size_t numBytes);
static int consWrite(File *file, const void *buf, size_t numBytes);


static VFSFileSystemOps _ops =
{
    .Stat = devFSStat,
    .Open = devFSOpen,
};


static FileOps _consoleOps = 
{
    .asyncRead = 1,
    .Read = consRead,
    .Write = consWrite
};

static VFSFileSystem _fs = {.ops = &_ops};

static DevFile* _devFiles = NULL; 

VFSFileSystem* getDevFS()
{
    return &_fs;
}

int DevFSAddDev(DevFile* file)
{
    KLOG_DEBUG("DevFSAddDev add device %s\n", file->name);
    HASH_ADD_STR(_devFiles, name, file);
    return 0;
}

static int devFSStat(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat)
{
    if(numPathSegments == 0) // root
    {
        stat->type = FileType_Dir;
        return 0;   

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

    int i =0;

    DevFile* f = NULL;
    DevFile* tmp = NULL;
    HASH_ITER(hh, _devFiles, f, tmp)
    {
        struct dirent *d = dirp + i;
        size_t fPos = i + file->readPos;
        
        snprintf(d->d_name, 256, "%s", f->name);
        acc += sizeof(struct dirent);
        d->d_off = acc;
        d->d_type = DT_DIR;
        d->d_reclen = sizeof(struct dirent);

        if(i >= numOfDirents)
        {
            break;
        }
        i++;
    }
    file->readPos += numOfDirents;
    return acc;
}

static FileOps _rootFOP = 
{
    .Read = _ReadDir,
    .asyncRead = 0
};

static int devFSOpen(VFSFileSystem *fs, const char *path, int mode, File *file)
{
    if(strcmp(path, "/") == 0)
    {

        file->ops = &_rootFOP;
        file->size = HASH_COUNT(_devFiles);
        return 0;
    }
    const char* p = path+1;
    
    DevFile* f = NULL;
    HASH_FIND_STR(_devFiles, p, f);

    if(!f)
    {
        return -ENOENT;
    }
    file->ops = f->ops;
    file->impl = f;
    file->size = -1;
    file->mode = O_RDWR;
    return 0;

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
    return -1;
}


static int consWrite(File *file, const void *buf, size_t numBytes)
{
    for(size_t i=0;i<numBytes;i++)
    {
        putchar(((char*)buf)[i]);
    }
    fflush(stdout);
    return numBytes;
}

