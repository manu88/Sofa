//
//  main.c
//  TestSysCalls
//
//  Created by Manuel Deneu on 26/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


#ifndef __APPLE__
#include <SysClient.h>
#endif

static int doOpenTests(void);
static int doGetPidTests(void);

static int doGetPidTests()
{
    errno = 0;
    assert(getpid() > 0);
    assert(errno == 0);
    
    errno = 0;
    assert(getppid() > 0);
    assert(errno == 0);
    return 1;
}
static int doOpenTests()
{
    errno = 0;
    assert(open(NULL, 0) == -1);
    assert(errno == EFAULT);
    
    errno = 0;
    assert(open("", 0) == -1);
    assert(errno == ENOENT);
    
    
    return 1;
}
int main(int argc, char * argv[])
{

#ifndef __APPLE__
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }
#endif
    assert(doGetPidTests());
    assert(doOpenTests());
    
    return 0;
}
