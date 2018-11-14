/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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


//
//  StringOperations.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <string.h>
#include <errno.h>
#include "StringOperations.h"


uint32_t StringHash(const char *str)
{
    uint32_t hash = 5381;
    uint32_t c;
    
    while ((c = (uint32_t)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}


void StringPrepend(char* s, const char* t)
{
    size_t len = strlen(t);
    size_t i;
    
    memmove(s + len, s, strlen(s) + 1);
    
    for (i = 0; i < len; ++i)
    {
        s[i] = t[i];
    }
}




char* GetRealPath(const char *path, const char *relativeTo, char resolved_path[] ,int* error)
{
    char copy_path[PATH_MAX];
    char link_path[PATH_MAX];
    char got_path[PATH_MAX];
    char *new_path = got_path;
    char *max_path;
    int readlinks = 0;
    int n;
    
    
    if (*path == '\0') {
        //__set_errno(ENOENT);
        return NULL;
    }
    /* Make a copy of the source path since we may need to modify it. */
    if (strlen(path) >= PATH_MAX - 2) {
        *error = -ENAMETOOLONG;
        return NULL;
    }
    strcpy(copy_path, path);
    path = copy_path;
    max_path = copy_path + PATH_MAX - 2;
    /* If it's a relative pathname use getcwd for starters. */
    if (*path != '/')
    {
        /* Ohoo... */
        //getcwd(new_path, PATH_MAX - 1);
        strcpy(new_path, relativeTo);
        //memcpy(new_path, relativeTo, strlen(relativeTo));
        new_path += strlen(new_path);
        if (new_path[-1] != '/')
            *new_path++ = '/';
    } else {
        *new_path++ = '/';
        path++;
    }
    /* Expand each slash-separated pathname component. */
    while (*path != '\0') {
        /* Ignore stray "/". */
        if (*path == '/') {
            path++;
            continue;
        }
        if (*path == '.') {
            /* Ignore ".". */
            if (path[1] == '\0' || path[1] == '/') {
                path++;
                continue;
            }
            if (path[1] == '.') {
                if (path[2] == '\0' || path[2] == '/') {
                    path += 2;
                    /* Ignore ".." at root. */
                    if (new_path == got_path + 1)
                        continue;
                    /* Handle ".." by backing up. */
                    while ((--new_path)[-1] != '/');
                    continue;
                }
            }
        }
        /* Safely copy the next pathname component. */
        while (*path != '\0' && *path != '/') {
            if (path > max_path) {
                *error = -ENAMETOOLONG;
                return NULL;
            }
            *new_path++ = *path++;
        }
//#ifdef S_IFLNK
#if 0
        /* Protect against infinite loops. */
        if (readlinks++ > MAX_READLINKS) {
            __set_errno(ELOOP);
            return NULL;
        }
        /* See if latest pathname component is a symlink. */
        *new_path = '\0';
        n = readlink(got_path, link_path, PATH_MAX - 1);
        if (n < 0) {
            /* EINVAL means the file exists but isn't a symlink. */
            if (errno != EINVAL) {
                /* Make sure it's null terminated. */
                *new_path = '\0';
                strcpy(resolved_path, got_path);
                return NULL;
            }
        } else {
            /* Note: readlink doesn't add the null byte. */
            link_path[n] = '\0';
            if (*link_path == '/')
            /* Start over for an absolute symlink. */
                new_path = got_path;
            else
            /* Otherwise back up over this component. */
                while (*(--new_path) != '/');
            /* Safe sex check. */
            if (strlen(path) + n >= PATH_MAX - 2) {
                __set_errno(ENAMETOOLONG);
                return NULL;
            }
            /* Insert symlink contents into path. */
            strcat(link_path, path);
            strcpy(copy_path, link_path);
            path = copy_path;
        }
#endif                            /* S_IFLNK */
        *new_path++ = '/';
    }
    /* Delete trailing slash but don't whomp a lone slash. */
    if (new_path != got_path + 1 && new_path[-1] == '/')
        new_path--;
    /* Make sure it's null terminated. */
    *new_path = '\0';
    strcpy(resolved_path, got_path);
    return resolved_path;
}
