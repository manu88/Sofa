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
//  FileServer.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once

#include <stdio.h>
#include "File.h"
#include "fs.h"
#include "Sofa.h"

#include "Bootstrap.h"

/* File System handler definition */
/*
typedef Inode* (* FileServerHandler_Open) (void* context, const char*pathname ,int flags, int *error) ;

typedef struct
{
    const char* prefix;
    FileServerHandler_Open onOpen;
    
    Inode inode;
    
} FileServerHandler;


int FileServerHandlerInit(FileServerHandler* hander , const char* name) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

 */


int FileServerInit(void) SOFA_UNIT_TESTABLE;


//int FileServerRegisterHandler( FileServerHandler* handler , const char* forPath) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
//int FileServerRemoveHandler( FileServerHandler* handler , const char* atPath) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

Inode* FileServerOpen(/*InitContext* context ,*/ const char*pathname , int flags , int *error) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
Inode* FileServerOpenRelativeTo( const char* pathname , int flags , int *error) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

Inode* FileServerGetRootNode(void ) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

// pass relativeTo = NULL to start from root ONLY IF the given path is absolute
Inode* FileServerGetINodeForPath(  const char* path ,const Inode* relativeTo) NO_NULL_ARGS(1,1) SOFA_UNIT_TESTABLE;


int FileServerAddNodeAtPath( Inode* node, const char* path) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
