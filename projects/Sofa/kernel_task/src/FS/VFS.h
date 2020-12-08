#pragma once
#include <utils/uthash.h>

typedef struct _VFS_File_Stat
{
    /* data */
}VFS_File_Stat;

typedef struct _VFSFileSystem VFSFileSystem;
/* Operations that can be performed on a mounted filesystem. */
typedef struct _VFSFileSystemOps 
{
    int (*Open)(VFSFileSystem *fs, const char *path, int mode);
    //int (*Create_Directory)(VFSFileSystem *fs, const char *path);
    //int (*Open_Directory)(VFSFileSystem *fs, const char *path, struct File **pDir);
    int (*Stat)(VFSFileSystem *fs, const char *path, VFS_File_Stat *stat);
    //int (*Sync)(struct Mount_Point *mountPoint);
    //int (*Delete)(struct Mount_Point *mountPoint, const char *path);
}VFSFileSystemOps;

typedef struct _VFSFileSystem
{
    char* mountPath;
    UT_hash_handle hh;

    VFSFileSystemOps *ops;
}VFSFileSystem;

int VFSInit(void);
int VFSMount(VFSFileSystem* fs, const char* mntPoint);

int VFSStat(const char *path, VFS_File_Stat *stat);