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

int FileServerInit(void) SOFA_UNIT_TESTABLE;

// Use FileServerOpenRelativeTo
SOFA_DEPRECATED("") File* FileServerOpen(/*InitContext* context ,*/ const char*pathname , int flags , int *error) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

File* FileServerOpenRelativeTo( const char* pathname ,const Inode* relativeTo , int flags , int *error) NO_NULL_ARGS(1,1) NO_NULL_ARGS(4,1) SOFA_UNIT_TESTABLE;

Inode* FileServerGetRootNode(void ) SOFA_UNIT_TESTABLE;

// pass relativeTo = NULL to start from root ONLY IF the given path is absolute
Inode* FileServerGetINodeForPath(  const char* path ,const Inode* relativeTo) NO_NULL_ARGS(1,1) SOFA_UNIT_TESTABLE;


int FileServerAddNodeAtPath( Inode* node, const char* path) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

Inode* FileServerCreateNode(const char* path, INodeType type, const Inode* relativeTo) NO_NULL_ARGS(1,1) SOFA_UNIT_TESTABLE;
int FileServerUnlinkNode(Inode* node) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int FileServer_DefaultOpen (Inode *node, int flags);
int FileServer_DefaultClose (Inode *node);

ssize_t FileServer_DefaultRead (Inode *node, char*buf  , size_t len);
ssize_t FileServer_DefaultLseek (Inode *node, size_t off, int whence);
ssize_t FileServer_DefaultWrite(Inode* node , const char* buff ,size_t len );
