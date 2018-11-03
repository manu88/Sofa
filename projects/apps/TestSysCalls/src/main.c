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
#include <sys/wait.h>

#ifndef __APPLE__
#include <SysClient.h>
#endif

static int doOpenTests(void);
static int doReadTests(void);
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

static int doReadTests()
{
    errno = 0;
    read(-1, NULL, 4);
    assert( errno == EBADF);
    

    return 1;
}


static int doWaitTests()
{
    errno = 0;
    int status = 0;
    assert(wait(&status) == -1);
    assert(errno == ECHILD);
    assert(status == 0);

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
    assert(doReadTests());
    assert(doWaitTests() );
    return 0;
}
