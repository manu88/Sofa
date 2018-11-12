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
#include <time.h>
#include <stdlib.h>
#include <string.h>

#ifndef __APPLE__
#include <SysClient.h>
#endif


static int doTimeTests(void);
static int doOpenTests(void);
static int doReadTests(void);
static int doGetPidTests(void);


static int doTimeTests()
{
    errno = 0;
    
    struct timespec ts = { 0 };
    int ret = clock_gettime(-1, &ts);
    assert(ret == -1);
    assert(errno == EINVAL);
    
    errno = 0;
    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(ret == 0);
    assert(errno == 0);
    

    errno = 0;
    struct timespec ts2 = { 0 };
    ret = clock_gettime(CLOCK_MONOTONIC, &ts2);
    assert(ret == 0);
    assert(errno == 0);
    assert( ts2.tv_nsec / 1000000000 + ts2.tv_sec >= ts.tv_nsec / 1000000000 + ts.tv_sec);
	
    return 1;
}

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

static int doGetCwdTests()
{
    errno = 0;
    char* rOk = getcwd(NULL, 0);
    assert(rOk);
    assert(errno == 0);
    printf("Path '%s'\n" , rOk);
    
    
    char* r;
    errno = 0;
    char buf[128];
    r = getcwd(buf, 1);
    assert(errno == ERANGE);
    assert(r == NULL);
    
    errno = 0;
    r = getcwd(buf, 128);
    assert(errno == 0);
    
    assert(strncmp(rOk, buf, strlen(rOk)) == 0) ;
    
    free(rOk);
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
    assert(doTimeTests());
    assert(doOpenTests());
    assert(doReadTests());
    assert(doWaitTests() );
    assert(doGetCwdTests() );
    return 0;
}
