//
//  ProcessTable_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 12/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include "ProcessTable_UnitTests.h"

#include "ProcessTable.h"
#include "FileServer.h"
#include "Bootstrap.h"

int ProcessTable_UnitTests()
{
    InitContext ctx;
    assert( ProcessTableInit() );
    assert(FileServerInit());
    
    assert(FileServerAddNodeAtPath(ProcessTableGetInode() , "/"));
    assert(FileServerGetINodeForPath("/proc" , NULL) == ProcessTableGetInode());
    
    Process initProcess;
    assert(ProcessInit(&initProcess));
    Process p1;
    assert(ProcessInit(&p1));
    
    assert(ProcessTableAppend(&p1));
    
    //assert(ProcessStart(&ctx, &p1, "app1", 0, &initProcess, 100) == 0);
    
    char strPID[32];
    sprintf(strPID, "/proc/%d", p1._pid);
    
    Inode* procNode = FileServerGetINodeForPath(strPID , NULL);
    assert(procNode);
    
    
    assert(ProcessTableRemove(&p1));
    procNode = FileServerGetINodeForPath(strPID , NULL);
    assert(procNode == NULL);
    
    return 1;
}
