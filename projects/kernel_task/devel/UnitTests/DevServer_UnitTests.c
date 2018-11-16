//
//  DevServer_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 09/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//
#include <assert.h>
#include <fcntl.h>
#include "DevServer_UnitTests.h"

#include "FileServer.h"
#include "DevServer.h"

#define GUARD_DATA 1234

typedef struct
{
    Inode node;
    
    int guardData;

} MyDriver;


int ConsoleOpenCalled = 0;

int ConsoleOpen (Inode* node, int flags)
{
    assert( node );
    MyDriver* console = (MyDriver*)node;
    
    assert( console );
    assert(console->guardData == GUARD_DATA);
    assert(node->userData == console);
    ConsoleOpenCalled++;
    return 0;
}

ssize_t ConsoleRead(Inode *node, char* buf  , size_t size )
{
    return 0;
}

ssize_t ConsoleWrite(Inode *node,  const char* buf ,size_t size)
{
    assert(buf);
    assert(size);
    return size;
}

ssize_t ConsoleLseek (Inode *node , size_t offset, int whence)
{
    return 0;
}

int DevServer_UnitTests()
{
    FileServerInit();
    
    assert(DevServerInit() );
    
    assert( FileServerAddNodeAtPath(DevServerGetInode(), "/"));
    assert( FileServerGetINodeForPath("/dev" ,NULL) != NULL);
    
    MyDriver consoleNode;
    consoleNode.guardData = GUARD_DATA;
    
    assert(InodeInit( (Inode*) &consoleNode, INodeType_File, "console"));
    assert(consoleNode.guardData == GUARD_DATA);
    
    consoleNode.node.userData = &consoleNode;
    
    INodeOperations consoleOps;
    consoleOps.Open = ConsoleOpen;
    
    FileOperations consoleFileOps;
    consoleFileOps.Lseek = ConsoleLseek;
    consoleFileOps.Write = ConsoleWrite;
    consoleFileOps.Read  = ConsoleRead;
    
    consoleNode.node.inodeOperations = &consoleOps;
    consoleNode.node.operations      = &consoleFileOps;
    
    assert( consoleNode.node.refCount == 1);
    assert(DevServerRegisterFile( (Inode*)&consoleNode ));
    assert( consoleNode.node.refCount == 2);
    
    assert(FileServerGetINodeForPath("/dev/console",NULL) == (Inode*) &consoleNode);
    assert(FileServerGetINodeForPath("/dev/consol",NULL) == NULL);
    
    
    
    int err = 0;
    Inode* consoleNodeOpened = FileServerOpen("/dev/console", O_RDWR, &err);
    assert( consoleNode.node.refCount == 3);
    
    assert(consoleNodeOpened);
    assert(consoleNodeOpened == (Inode*) &consoleNode);
    
    assert( consoleNodeOpened->operations      == &consoleFileOps);
    assert( consoleNodeOpened->inodeOperations == &consoleOps);
    
    assert(ConsoleOpenCalled == 1);
    
    const char writeBuf[] = "Hello";
    ssize_t writeRet = consoleNodeOpened->operations->Write(consoleNodeOpened , writeBuf , strlen(writeBuf));
    assert(writeRet == strlen(writeBuf) );
    
    
    // now remove the device
    
    assert(FileServerGetINodeForPath("/dev/console",NULL) == (Inode*) &consoleNode);
    
    
    assert(DevServerRemoveFile( (Inode*) &consoleNode));
    assert( consoleNode.node.refCount == 2);
    
    
    assert(FileServerGetINodeForPath("/dev/console",NULL) == NULL);
    
    return 1;
}
