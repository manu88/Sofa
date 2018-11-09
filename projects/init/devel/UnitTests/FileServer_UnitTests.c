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
//  FileServer_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 03/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include "FileServer_UnitTests.h"

#include "FileServer.h"

static int cpioCalled = 0;
static int cpioReturnsValue = 0;

static Inode* CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
    cpioCalled = 1;
    
    if(cpioReturnsValue)
    {
        return InodeAlloc();
    }
    return NULL;
}


static int Inode_tests()
{
    Inode* node = InodeAlloc();
    assert(node);
    assert(node->_parent == NULL);
    assert(node->operations == NULL);
    assert(node->pos  == 0);
    assert(node->size == 0);
    assert(node->userData == NULL);
    
    InodeRelease(node);
    return 1;
}

int FileServer_UnitTests()
{
    assert(Inode_tests());
    assert(FileServerInit());
    
    assert(FileServerGetRootNode());
    assert(InodeGetChildrenCount(FileServerGetRootNode()) == 0);
    
    FileServerHandler cpioHandler;
    cpioHandler.prefix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    assert(FileServerRegisterHandler(&cpioHandler ,"/cpio/") );
    assert(FileServerRegisterHandler(&cpioHandler ,"/cpio/")  == 0); // second time must fail
    
    // we have now cpio node
    assert(InodeGetChildrenCount(FileServerGetRootNode()) == 1);
    
    int err = 0;
    
    int fakeContextSoThatClangIsHappy = 1;
    assert(FileServerOpen(&fakeContextSoThatClangIsHappy, "/cpio/test", 0 , &err) == NULL );
    
    assert(cpioCalled == 1);
    cpioReturnsValue = 1;
    
    
    Inode* node = FileServerOpen(&fakeContextSoThatClangIsHappy, "/cpio/test", 0 , &err);
    assert( node );
    
    
    InodeRelease(node);
    
    return 1;
}
