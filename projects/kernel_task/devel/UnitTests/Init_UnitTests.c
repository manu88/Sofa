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
//  Init_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 03/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include "FileServer_UnitTests.h"
#include "Init_UnitTests.h"
#include "ProcessTable.h"
#include "TimersWheel_UnitTests.h"
#include "DriverKit_UnitTests.h"
#include "DevServer_UnitTests.h"
#include "ProcessTable_UnitTests.h"
#include "Other_UnitTests.h"

static int Test_ProcessInstance(Process* p)
{
    assert(ProcessGetNumChildren(p) == 0);
    assert(ProcessGetChildByPID(p, 0) == NULL);
    assert(ProcessGetChildByPID(p, 1) == NULL);
    
    assert(p->_parent == NULL);
    return 1;
}

static int Test_Process()
{
    Process* p1 = ProcessAlloc();
    assert(Test_ProcessInstance(p1));
    
    Process p2;
    
    assert(ProcessInit(&p2));
    assert(Test_ProcessInstance(&p2));
    
    p1->_pid = 1;
    p2._pid = 2;
    
    assert(ProcessSetParentShip(p1, &p2) == 0); // 0 means no error on this one
    assert(p2._parent == p1);
    assert(ProcessGetChildByPID(p1, 2) == &p2);
    
    ProcessRelease(p1);
    ProcessDeInit(&p2);
    return 1;
}

static int Test_ProcessTable()
{
    assert(ProcessTableInit()) ;
    
    assert(ProcessTableGetCount() == 0);
    
    Process* p1 = ProcessAlloc();
    assert(p1);
    assert(p1->_pid == 0);
    assert(p1->_parent == 0);
    
    p1->_pid = 1;
    assert(ProcessTableAppend(p1));
    assert(ProcessTableGetCount() == 1);
    assert(ProcessTableGetByPID(0) == NULL);
    assert(ProcessTableGetByPID(-1) == NULL);
    
    assert(ProcessTableGetByPID(1) == p1);
    
    ProcessTableRemove(p1);
    assert(ProcessTableGetCount() == 0);
    assert(ProcessTableGetByPID(1) == NULL);
    
    ProcessRelease(p1);
    
    
    
    return 1;
}

int doInit_UnitTests()
{
    assert(GetRealPath_tests() );
    assert(TimersWheel_UnitTests() );
    assert(FileServer_UnitTests());
    assert(Test_Process());
    assert(Test_ProcessTable());
    assert(DriverKit_UnitTests() );
    assert(DevServer_UnitTests() );
    assert(ProcessTable_UnitTests() );
    return 1;
}
