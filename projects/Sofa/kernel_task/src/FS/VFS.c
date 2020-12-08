#include "VFS.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#define MAX_PREFIX_LEN 16

static VFSFileSystem* _fileSystems = NULL;

/*
 * Unpack a path into prefix and suffix.
 * The prefix determines which mounted filesystem
 * the path resides in.  The suffix is the path within
 * the filesystem.
 * Params:
 *   path - the complete path
 *   prefix - buffer where prefix will be stored
 *   pSuffix - stores the pointer to the suffix part of path
 * Returns: true if path is valid, false if not
 */
static uint8_t Unpack_Path(const char *path, char *prefix, const char **pSuffix)
{
    char *slash;
    size_t pfxLen;

    printf("path=%s\n", path);

    /* Path must start with '/' */
    if (*path != '/')
	    return 0;
    ++path;

    /* Look for the initial slash. */
    slash = strchr(path, '/');
    if (slash == 0) {
	/*
	 * Special case: path of the form "/prefix".
	 * It resolves to the root directory of
	 * the filesystem mounted on the prefix.
	 */
	pfxLen = strlen(path);
	if (pfxLen == 0 || pfxLen > MAX_PREFIX_LEN)
	    return 0;
	strcpy(prefix, path);
	*pSuffix = "/";
    } else {
	/*
	 * Determine length of file prefix.
	 * It needs to be non-zero, but less than MAX_PREFIX_LEN.
	 */
	pfxLen = slash - path;
	if (pfxLen == 0 || pfxLen > MAX_PREFIX_LEN)
	    return 0;

	/* Format the path prefix as a string */
	memcpy(prefix, path, pfxLen);
	prefix[pfxLen] = '\0';

	/*
	 * Set pointer to "suffix", i.e., the rest of the path
	 * after the prefix
	 */
	*pSuffix = slash;
    }

//    printf("prefix=%s, suffix=%s\n", prefix, *pSuffix);
    assert(**pSuffix == '/');

    return 1;
}

VFSFileSystem* _GetFileSystem(const char* mountPath)
{
    VFSFileSystem* fs = NULL;
    HASH_FIND_STR(_fileSystems, mountPath, fs);
    return fs;
}

int VFSMount(VFSFileSystem* fs, const char* mntPoint)
{
    assert(fs);
    assert(mntPoint);

    /* Skip leading slash character(s) */
    while (*mntPoint == '/')
	++mntPoint;

    if (strlen(mntPoint) > MAX_PREFIX_LEN)
    {
	    return ENAMETOOLONG;
    }

    if(_GetFileSystem(mntPoint))
    {
        return EEXIST;
    }
    fs->mountPath = strdup(mntPoint);

    HASH_ADD_STR(_fileSystems, mountPath, fs);

    return 0;
}

int VFSInit()
{
    return 0;
}

int VFSStat(const char *path, VFS_File_Stat *stat)
{
    char prefix[MAX_PREFIX_LEN + 1];
    const char *suffix;

    if(strcmp(path, "/") == 0)
    {
        VFSFileSystem* fs = NULL;
        VFSFileSystem* tmp = NULL;
        HASH_ITER(hh, _fileSystems, fs, tmp)
        {
            printf("%s\n", fs->mountPath);
        }

        return 0;
    }
    
    if (!Unpack_Path(path, prefix, &suffix))
    {
	    return ENOENT;
    }


    VFSFileSystem* fs =  _GetFileSystem(prefix);
    if(fs == NULL)
    {
        printf("No fs for '%s'\n", path);
        return ENOENT;
    }
    printf("Got FS for '%s' forward req\n", path);
    fs->ops->Stat(fs, suffix, stat);

    return 0;
}