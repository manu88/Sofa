//
//  Other_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 15/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include "Other_UnitTests.h"
#include <assert.h>
#include "StringOperations.h"
#include <string.h>


static void GetRealPath_t1()
{
    const char path[] = "someFile";
    const char currentDir[] = "/dev";
    
    char resolved[PATH_MAX] = {0};
    int err = 0;
    GetRealPath(path, currentDir, resolved, &err);
    
    assert(strcmp("/dev/someFile", resolved) == 0);
    assert(err == 0);
}

static void GetRealPath_t2()
{
    const char path[] = "someFile";
    const char currentDir[] = "/";
    
    char resolved[PATH_MAX] = {0};
    int err = 0;
    GetRealPath(path, currentDir, resolved, &err);
    
    assert(strcmp("/someFile", resolved) == 0);
    assert(err == 0);
}

static void GetRealPath_t3()
{
    const char path[] = "/dev/lolz/someFile";
    const char currentDir[] = "/";
    
    char resolved[PATH_MAX] = {0};
    int err = 0;
    GetRealPath(path, currentDir, resolved, &err);
    
    assert(strcmp("/dev/lolz/someFile", resolved) == 0);
    assert(err == 0);
}

static void GetRealPath_t4()
{
    const char path[] = "../test/../someFile";
    const char currentDir[] = "/dev/test/";
    
    char resolved[PATH_MAX] = {0};
    int err = 0;
    GetRealPath(path, currentDir, resolved, &err);
    
    assert(strcmp("/dev/someFile", resolved) == 0);
    assert(err == 0);
}

int GetRealPath_tests()
{
    GetRealPath_t1();
    GetRealPath_t2();
    GetRealPath_t3();
    GetRealPath_t4();
    
    return 1;
}
