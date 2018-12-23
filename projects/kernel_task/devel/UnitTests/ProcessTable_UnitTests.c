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
    assert( ProcessTableInit() );
    assert(FileServerInit());
    
    
    assert(FileServerAddNodeAtPath(ProcessTableGetInode() , "/"));
    assert(FileServerGetINodeForPath("/proc" , NULL) == ProcessTableGetInode());
    
    Process initProcess;
    assert(ProcessInit(&initProcess));
    assert(initProcess.cmdLine == NULL);
    assert( initProcess._procStats.numSysCalls == 0);
    assert(initProcess._pid == 0);
    assert(initProcess._prio == 0);
    assert(ProcessGetNumChildren(&initProcess) == 0);
    
    Process p1;
    assert(ProcessInit(&p1));
    assert(initProcess.cmdLine == NULL);
    assert(initProcess.startTime == 0);
    //assert(ProcessTableAppend(&p1));
    
    KernelTaskContext ctx;
    assert(ProcessTableContains(&p1) == 0);
    ProcessTableAddAndStart(&ctx, &p1, "initCmd", 0, &initProcess, 255);
    
    assert(ProcessTableContains(&p1));
    assert( strcmp( p1.cmdLine , "initCmd" ) == 0 );
    assert(initProcess.cmdLine == NULL);
    //assert(ProcessStart(&ctx, &p1, "app1", 0, &initProcess, 100) == 0);
    
    char strPID[32];
    sprintf(strPID, "/proc/%d", p1._pid);
    
    Inode* procNode = FileServerGetINodeForPath(strPID , NULL);
    assert(procNode);
    assert(procNode->type == INodeType_Folder);
    
    
    char strPID2[128];
    sprintf(strPID2, "/proc/%d/status", p1._pid);
    Inode* statusNode = FileServerGetINodeForPath(strPID2, NULL);
    assert(statusNode);
    
    sprintf(strPID2, "/proc/%d/cmdline", p1._pid);
    Inode* cmdLineNode = FileServerGetINodeForPath(strPID2, NULL);
    assert(cmdLineNode);
    
    
    assert(ProcessTableRemove(&p1));
    assert(ProcessTableContains(&p1) == 0);
    procNode = FileServerGetINodeForPath(strPID , NULL);
    assert(procNode == NULL);
    
    return 1;
}
