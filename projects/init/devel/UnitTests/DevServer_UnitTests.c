//
//  DevServer_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 09/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//
#include <assert.h>

#include "DevServer_UnitTests.h"

#include "FileServer.h"
#include "DevServer.h"

int DevServer_UnitTests()
{
    FileServerInit();
    
    assert(DevServerInit() );
    
    assert( FileServerAddNodeAtPath(DevServerGetInode(), "/"));
    assert( FileServerGetINodeForPath("/dev") != NULL);
    
    Inode consoleNode;
    assert(InodeInit(&consoleNode, INodeType_File, "console"));
    
    INodeOperations consoleOps;
    consoleNode.inodeOperations = &consoleOps;
    
    assert(DevServerRegisterFile( &consoleNode ));
    
    assert(FileServerGetINodeForPath("/dev/console") == &consoleNode);
    assert(FileServerGetINodeForPath("/dev/consol") == NULL);
    
    return 1;
}
