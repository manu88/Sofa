#pragma once
#include <utils/uthash.h>
#include <sys/types.h>

typedef struct _VFS_File_Stat
{
    /* data */
}VFS_File_Stat;

typedef struct _VFSFileSystem VFSFileSystem;
typedef struct _File File;
/* Operations that can be performed on a mounted filesystem. */
typedef struct _VFSFileSystemOps 
{
    int (*Open)(VFSFileSystem *fs, const char *path, int mode, File *file);
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

typedef struct _File File;

/* Operations that can be performed on a File. */
typedef struct _FileOps {
    //int (*FStat)(struct File *file, struct VFS_File_Stat *stat);
    int (*Read)(File *file, void *buf, size_t numBytes);
    //int (*Write)(struct File *file, void *buf, ulong_t numBytes);
    int (*Seek)(File *file, size_t pos);
    int (*Close)(File *file);
    //int (*Read_Entry)(struct File *dir, struct VFS_Dir_Entry *entry);  /* Read next directory entry. */
}FileOps;

typedef struct _File
{
    int mode;
    size_t readPos;
    size_t size;
    FileOps* ops;

    void* impl;
}File;

int VFSInit(void);
int VFSMount(VFSFileSystem* fs, const char* mntPoint);

int VFSStat(const char *path, VFS_File_Stat *stat);
int VFSOpen(const char* path, int mode, File* file);

ssize_t VFSRead(File* file, char* buf, size_t sizeToRead);
int VFSClose(File* file);
int VFSSeek(File* file, size_t pos);