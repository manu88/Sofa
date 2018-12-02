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

#include "Null.h"
#include "../DevServer.h"

typedef struct
{
    Inode node;
    
} NULLDev;


static NULLDev _dev;
static const char _nullName[] = "null";

static ssize_t NULLWrite (struct _inode *node,  const char*buffer ,size_t size)
{
    return (ssize_t) size;
}

static FileOperations  nullFileOps = {FileOperation_NoRead , NULLWrite, FileOperation_NoLseek };

int DevNullInit()
{
    if(InodeInit(&_dev.node, INodeType_File, _nullName))
    {
        _dev.node.operations = &nullFileOps;
        return DevServerRegisterFile(&_dev.node);
    }
	return 0;
}
