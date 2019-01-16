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

#include "FileSystem.h"


typedef struct
{
    struct kset obj;
    
} FileSystem;

static char fsObjectName[] = "fs";

static struct kobj_type _fsType =
{
    NULL, // release
};

static FileSystem _fs;

OSError FileSystemInit()
{
    kset_init(&_fs.obj);
    
    _fs.obj.obj.k_name = fsObjectName;
    _fs.obj.obj.ktype  = &_fsType;
    
    return OSError_None;
}

struct kobject* FileSystemGetObject()
{
    return &_fs.obj.obj;
}
