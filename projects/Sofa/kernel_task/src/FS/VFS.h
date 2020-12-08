#pragma once

typedef struct _VFS_File_Stat
{
    /* data */
}VFS_File_Stat;

int VFSStat(const char *path, VFS_File_Stat *stat);