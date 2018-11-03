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
    
    
    FileServerHandler cpioHandler;
    cpioHandler.perfix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    assert(FileServerRegisterHandler(&cpioHandler) );
    assert(FileServerRegisterHandler(&cpioHandler)  == 0); // second time must fail
    
    
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
