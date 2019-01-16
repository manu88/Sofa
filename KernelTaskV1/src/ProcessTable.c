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

#include "ProcessTable.h"

typedef struct
{
    struct kset obj;
    
} ProcessTable;

static char procTableObjectName[] = "proc";

static struct kobj_type _ProcessTableType =
{
    NULL, // release
};

static ProcessTable _procTable;

OSError ProcessTableInit()
{
    kset_init(&_procTable.obj);
    _procTable.obj.obj.k_name = procTableObjectName;
    _procTable.obj.obj.ktype  = &_ProcessTableType;
    
    return OSError_None;
}

struct kobject* ProcessTableGetObject()
{
    return &_procTable.obj.obj;
}
