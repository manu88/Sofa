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
#pragma once
#include <utils/uthash.h>
#include <sys/types.h>

typedef enum
{
    FileType_Regular,
    FileType_Dir,
}FileType;

typedef struct _VFS_File_Stat
{
    FileType type;
}VFS_File_Stat;

typedef struct _VFSFileSystem VFSFileSystem;
typedef struct _File File;
/* Operations that can be performed on a mounted filesystem. */
typedef struct _VFSFileSystemOps 
{
    int (*Open)(VFSFileSystem *fs, const char *path, int mode, File *file);
    //int (*Create_Directory)(VFSFileSystem *fs, const char *path);
    //int (*Open_Directory)(VFSFileSystem *fs, const char *path, struct File **pDir);
    int (*Stat)(VFSFileSystem *fs, const char **path, int numPathSegments, VFS_File_Stat *stat);
    //int (*Sync)(struct Mount_Point *mountPoint);
    //int (*Delete)(struct Mount_Point *mountPoint, const char *path);
}VFSFileSystemOps;

typedef struct _VFSFileSystem
{
    VFSFileSystemOps *ops;

    void* data;
}VFSFileSystem;


typedef struct _VFSMountPoint
{
    VFSFileSystem *fs;
    char* mountPath;
    UT_hash_handle hh;
} VFSMountPoint;

typedef struct _File File;

/* Operations that can be performed on a File. */
typedef struct _FileOps {
    //int (*FStat)(struct File *file, struct VFS_File_Stat *stat);
    int (*Read)(File *file, void *buf, size_t numBytes);
    int (*Write)(File *file, const void *buf, size_t numBytes);
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
VFSMountPoint* VFSMount(VFSFileSystem* fs, const char* mntPoint, int*err);

int VFSStat(const char *path, VFS_File_Stat *stat);
int VFSOpen(const char* path, int mode, File* file);

ssize_t VFSRead(File* file, char* buf, size_t sizeToRead);
ssize_t VFSWrite(File* file, const char* buf, size_t sizeToWrite);

int VFSClose(File* file);
int VFSSeek(File* file, size_t pos);