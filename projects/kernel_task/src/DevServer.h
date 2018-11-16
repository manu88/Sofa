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
//  DevServer.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once

#include "FileServer.h"
#include "fs.h"
#include "Sofa.h"

struct _DeviceOperations
{
    FileOperations fileOps;
    
    
    void* userContext;
};

typedef  struct _DeviceOperations DeviceOperations;



Inode* DevServerGetInode(void) SOFA_UNIT_TESTABLE;

int DevServerInit(void);

// will retain the node
int DevServerRegisterFile( Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

// will release the node
int DevServerRemoveFile( Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;



