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
#include "CpioServer.h"

static int FileServer_OperationTests(void);
static int FileServer_WalkTests(void);



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
    
    assert(FileServerGetINodeForPath("",NULL) == NULL);
    assert(FileServerGetINodeForPath("/",NULL) == FileServerGetRootNode() );
    
    const Inode* node = FileServerGetINodeForPath("/lolz",NULL);
    assert(node == NULL );
    
    assert(FileServerGetRootNode());
    assert(FileServerGetRootNode()->_parent == FileServerGetRootNode() );
    
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
    assert(FileServerGetINodeForPath("/proc",NULL) == procNode);
    assert(FileServerGetINodeForPath("/proc/",NULL) == procNode);
    
    Inode* devNode = InodeAlloc(INodeType_Folder, "dev");
    assert(devNode);
    
    assert(InodeAddChild( FileServerGetRootNode(), devNode));
    assert(devNode->_parent == FileServerGetRootNode());
    assert(FileServerGetINodeForPath("/dev",NULL) == devNode);
    assert(FileServerGetINodeForPath("/dev/",NULL) == devNode);
    assert(FileServerGetINodeForPath("//dev/",NULL) == devNode);
    
    int accumIter = 0;
    
    Inode* c = NULL;
    Inode* tempChild = NULL;
    InodeForEachChildren(FileServerGetRootNode(), c, tempChild)
    {
        assert(c);
        
        accumIter++;
    }
    assert( InodeGetChildrenCount(FileServerGetRootNode()) == accumIter);
    
    
    assert(InodeRemoveChild(FileServerGetRootNode(), devNode));
    assert(InodeRemoveChild(FileServerGetRootNode(), procNode));
    

    assert( InodeGetChildrenCount(FileServerGetRootNode()) == 0 );
    
    assert(procNode->refCount == 1);
    assert(devNode->refCount == 1);
    assert(InodeRelease(procNode));
    assert(InodeRelease(devNode));
    free(procNode);
    free(devNode);
    
    
    assert(FileServer_OperationTests() );
    assert(FileServer_WalkTests() );
    
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
    Inode* procNodeRetained = FileServerOpen( "/proc/", OpenFlags, &error);
    assert(procNodeRetained == NULL);
    assert(error == ProcOpenReturn);
    assert(procNode.refCount == 1); // refcount unchanged
    
    // 2nd time ok
    ProcOpenReturn = 0;
    procNodeRetained = FileServerOpen( "/proc/", OpenFlags, &error);
    assert(procNodeRetained == &procNode);
    assert(procNode.refCount == 2);
    
    assert(InodeRelease(procNodeRetained) == 0);
    
    assert(InodeGetChildrenCount(procNodeRetained) == 0);
    
    
    /* CPIO Part */
    
    assert( CPIOServerInit() );
    
    assert(FileServerAddNodeAtPath( CPIOServerGetINode() , "/"));
    assert( FileServerGetINodeForPath("/cpio",NULL) );
    assert( FileServerGetINodeForPath("/cpio/",NULL) );
    
    int err = 0;
    Inode* testNode = FileServerOpen( "/cpio/test", 1, &err);
    
    
    return 1;
}


static int FileServer_WalkTests()
{
    FileServerInit();
    
    Inode f1;
    assert(InodeInit(&f1, INodeType_Folder, "folder1"));
    assert(FileServerAddNodeAtPath(&f1, "/"));
    
    Inode* currentDir = FileServerGetINodeForPath(".", FileServerGetRootNode() );
    assert(currentDir == FileServerGetRootNode() );
    
    Inode* newDir = FileServerGetINodeForPath("/folder1/../", currentDir) ;
    assert(newDir== currentDir);
    
    newDir = FileServerGetINodeForPath("/folder1/./", currentDir);
    assert(newDir == &f1);
    
    assert(newDir->_parent == FileServerGetRootNode());
    
    newDir = FileServerGetINodeForPath("..", newDir);
    assert(newDir == FileServerGetRootNode());
    
    
    newDir = FileServerGetINodeForPath("./folder1/", NULL);
    assert( newDir == &f1);
    
    Inode f2;
    assert(InodeInit(&f2, INodeType_Folder, "folder2"));
    assert(FileServerAddNodeAtPath(&f2, "/folder1"));
    
    Inode file1;
    assert(InodeInit(&file1, INodeType_File, "file1"));
    assert(FileServerAddNodeAtPath(&file1, "/folder1"));
    
    Inode f3;
    assert(InodeInit(&f3, INodeType_Folder, "folder3"));
    assert(FileServerAddNodeAtPath(&f3, "/"));
    
    newDir = FileServerGetINodeForPath("folder2", newDir);
    assert(newDir == &f2);
    
    newDir = FileServerGetINodeForPath("../..", newDir);
    assert(newDir == FileServerGetRootNode() );
    
    
    assert( strcmp("folder1", f1.name) == 0);
    assert( strcmp("folder2", f2.name) == 0);
    
    assert(f2._parent == &f1);
    assert(f1._parent == FileServerGetRootNode() );
    
    Inode* filePtr = FileServerGetINodeForPath("folder1/file1" , newDir);
    assert( filePtr == &file1);
    
    InodePrintTree(FileServerGetRootNode());
    return 1;
}
