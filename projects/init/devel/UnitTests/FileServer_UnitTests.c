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
#include <string.h>
#include "FileServer_UnitTests.h"
#include <errno.h>
#include "FileServer.h"


static int FileServer_OperationTests(void);

static int cpioCalled = 0;
static int cpioReturnsValue = 0;

static Inode* CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
    cpioCalled = 1;
    
    if(cpioReturnsValue)
    {
        Inode* node = InodeAlloc(INodeType_File , pathname);

        return node;
    }
    return NULL;
}


static int Inode_tests()
{
    Inode* node = InodeAlloc(INodeType_File , "foo");
    assert(node);
    assert(node->refCount == 1);
    assert(node->type == INodeType_File);
    assert(node->_parent == NULL);
    assert(node->operations == NULL);
    assert(node->pos  == 0);
    assert(node->size == 0);
    assert(node->userData == NULL);
    
    
    assert(InodeGetChildByName(node, "lol") == NULL);
    Inode* c1 = InodeAlloc(INodeType_Folder , "bar");
    
    assert( InodeAddChild(node, c1) == 0); // must fail because node is NOT a folder;
    assert(InodeGetChildByName(node, c1->name) == NULL);
    
    node->type = INodeType_Folder;
    assert( InodeAddChild(node, c1) ); // Now it works
    assert(InodeGetChildByName(node, c1->name) == c1);
    
    Inode* c2_wrong = InodeAlloc(INodeType_Folder , "bar");
    assert( InodeAddChild(node, c2_wrong) == 0); // must fail because c1 & c2 have the same name
    InodeRelease(c2_wrong);
    
    char buf[128] = {0};
    
    assert( InodeGetAbsolutePath(c1, buf, 128) );
    assert(strcmp(buf, "foo/bar/") == 0);
    
    Inode* c3 = InodeAlloc(INodeType_File , "file");
    assert( InodeAddChild(c1, c3));
    assert(c3->_parent == c1);
    assert(c1->_parent == node);
    assert(node->_parent == NULL);
    assert( InodeGetAbsolutePath(c3, buf, 128) );
    assert(strcmp(buf, "foo/bar/file") == 0);
    
    InodeRelease(node);
    InodeRelease(c1);
    return 1;
}

int FileServer_UnitTests()
{
    assert(Inode_tests());
    //assert(FileServerInit());
    
    assert(FileServerGetINodeForPath("") == NULL);
    assert(FileServerGetINodeForPath("/") == FileServerGetRootNode() );
    
    assert(FileServerGetINodeForPath("/lolz") == NULL );
    
    assert(FileServerGetRootNode());
    assert(FileServerGetRootNode()->_parent == NULL);
    
    assert(FileServerGetRootNode()->refCount == 1);
    InodeRetain(FileServerGetRootNode());
    assert(FileServerGetRootNode()->refCount == 2);
    assert(InodeRelease(FileServerGetRootNode() ) == 0);
    assert(FileServerGetRootNode()->refCount == 1);
    
    
    assert(InodeGetChildrenCount(FileServerGetRootNode()) == 0);
    
    char nameBuffer[128] = {0};
    
    assert(InodeGetAbsolutePath(FileServerGetRootNode(), nameBuffer, 0) == -1);
    ssize_t ret = InodeGetAbsolutePath(FileServerGetRootNode(), nameBuffer, 128);
    assert(ret == 1);
    assert(strcmp(nameBuffer, "/") == 0);
    
    
    Inode* procNode = InodeAlloc(INodeType_Folder, "proc");
    assert(procNode);
    
    assert(InodeAddChild( FileServerGetRootNode(), procNode));
    assert(procNode->_parent == FileServerGetRootNode());
    assert(FileServerGetINodeForPath("/proc") == procNode);
    assert(FileServerGetINodeForPath("/proc/") == procNode);
    
    Inode* devNode = InodeAlloc(INodeType_Folder, "dev");
    assert(devNode);
    
    assert(InodeAddChild( FileServerGetRootNode(), devNode));
    assert(devNode->_parent == FileServerGetRootNode());
    assert(FileServerGetINodeForPath("/dev") == devNode);
    assert(FileServerGetINodeForPath("/dev/") == devNode);
    assert(FileServerGetINodeForPath("//dev/") == devNode);
    
    assert(InodeRemoveChild(FileServerGetRootNode(), devNode));
    assert(InodeRemoveChild(FileServerGetRootNode(), procNode));
    

    assert( InodeGetChildrenCount(FileServerGetRootNode()) == 0 );
    
    
    assert(FileServer_OperationTests() );
    
    return 1;
}

static int OpenFlags = 123;



static int ProcOpenReturn = -ENXIO;

static int ProcOpen (struct _inode *node, int flags)
{
    assert(flags == OpenFlags);
    return ProcOpenReturn;
}

static int FileServer_OperationTests()
{
    InitContext ctx;
    FileServerInit();
    
    
    INodeOperations inodeops;
    inodeops.Open = ProcOpen;
    
    Inode procNode;
    assert(InodeInit(&procNode, INodeType_Folder, "proc"));
    
    procNode.inodeOperations = &inodeops;
    
    assert(FileServerAddNodeAtPath(&procNode, "/"));
    
    assert(InodeGetChildrenCount(FileServerGetRootNode()) == 1 );
    
    int error = 0;
    
    // 1st time fails
    Inode* procNodeRetained = FileServerOpen(&ctx, "/proc/", OpenFlags, &error);
    assert(procNodeRetained == NULL);
    assert(error == ProcOpenReturn);
    assert(procNode.refCount == 1); // refcount unchanged
    
    // 2nd time ok
    ProcOpenReturn = 0;
    procNodeRetained = FileServerOpen(&ctx, "/proc/", OpenFlags, &error);
    assert(procNodeRetained == &procNode);
    assert(procNode.refCount == 2);
    
    assert(InodeRelease(procNodeRetained) == 0);
    
    assert(InodeGetChildrenCount(procNodeRetained) == 0);
    
    return 1;
}
