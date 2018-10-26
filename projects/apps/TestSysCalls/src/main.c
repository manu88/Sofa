//
//  main.c
//  TestSysCalls
//
//  Created by Manuel Deneu on 26/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>

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
int main(int argc, const char * argv[])
{
    assert(doGetPidTests());
    assert(doOpenTests());
    
    return 0;
}
