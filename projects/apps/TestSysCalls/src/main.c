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
#include <dirent.h>
#include <sys/stat.h>

#ifndef __APPLE__
#include <SysClient.h>
#endif


static int doTimeTests(void);
static int doOpenTests(void);
static int doCloseTests(void);
static int doOpenTests2(void);
static int doReadTests(void);
static int doGetPidTests(void);

static int doDevNullTests(void);

static int doLsTests(void);
static int doLsTests2(void);

static int doCreateAndStatFileTest(void);


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
    assert(getppid() >= 0);
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

static int doOpenTests2()
{

    int fd = open("/" , O_RDONLY);
    assert(fd >= 0);
    char b[128];
    
    ssize_t ret = read(fd, b, 128);

    assert(ret == -1);
    assert(errno == EISDIR);

    int status = fcntl(fd, F_GETFD);
    assert(status == 0); // fd still valid 'cause not closed
    
    close(fd);
    
    status = fcntl(fd, F_GETFD);
    assert(status == -1); // fd invalid 'cause closed
    

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

    char* r;
    errno = 0;
    char buf[128];
    memset(buf, 0, 128);

    
/*
    r = getcwd(buf, 1);
    assert(errno == ERANGE);
    assert(r == NULL);
*/    
    errno = 0;
    memset(buf, 0, 128);

    
    r = getcwd(buf, 128);
    assert(errno == 0);
    assert(strncmp(rOk, buf, strlen(rOk)) == 0) ;

/*    
    errno = 0;
    memset(buf, 0, 128);

    r = getcwd(buf, strlen(rOk) - 2);
    assert(errno == ERANGE);
  */  
    
    free(rOk);
    return 1;
}

static int doChdirTests()
{

    errno = 0;
    assert(chdir(NULL) == -1);
    assert(errno == EFAULT);

    errno = 0;
    assert(chdir("") == -1);
    assert(errno == ENOENT);
    

    errno = 0;
    assert(chdir("prout") == -1);
    assert(errno == ENOENT);
    
    errno = 0;
    assert(chdir("/") == 0);
    assert(errno == 0);
    
    char *currentDir = getcwd(NULL, 0);
    assert(currentDir);
    assert(strcmp(currentDir, "/") == 0);
    
    free(currentDir);
    //printf("ret %i errno %i\n" , ret , errno);
    return 1;
}

static int doUnlinkTests()
{
    
    int ret = 0;
    
    errno = 0;
    ret = unlink(NULL);
    assert(ret == - 1);
    assert(errno == EFAULT);
    
    errno = 0;
    ret = unlink("foo123");// should not exist
    assert(ret == - 1);
    assert(errno == ENOENT);
    
    errno = 0;
    const char filename[]  = "myFile";
    int fd2 = open (filename, O_RDWR|O_CREAT);
    assert( fd2 >= 0);
    assert(errno == 0);
    
    errno = 0;
    ret = close(fd2);
    assert(ret == 0);
    assert(errno == 0);
    
    errno = 0;
    ret = unlink(filename);
    assert(ret == 0);
    assert(errno == 0);
    
    // 2nd time must fail
    errno = 0;
    ret = unlink(filename);

    assert(ret == -1);
    assert(errno == ENOENT);
    

    
    return 1;
}

static int doLsTests()
{
    
    errno = 0;
    assert(chdir("/") == 0);
    assert(errno == 0);
    
    struct dirent *dir;
    DIR *d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
	    assert(dir->d_name);
//            printf("dir entry '%s'\n", dir->d_name);
        }
        closedir(d);
    }
    
    return 1;
}

static int doLsTests2()
{
    return 1;
}


static int doCreateAndStatFileTest()
{

    
    errno = 0;
    int fd = open("newFile", O_WRONLY | O_APPEND | O_CREAT , 0644);
    assert(errno == 0);
    assert(fd >= 0);
    
    
    // test stat
    errno = 0;
    struct stat s;
    int ret = stat(NULL, &s);
    assert(errno == EFAULT);
    assert( ret == -1);
    
    errno = 0;
    ret = stat("", NULL); // non existing file
    assert(errno == ENOENT);
    assert( ret == -1);
    
    errno = 0;
    ret = stat("newFileThatIsInexistant", &s);
    assert( errno == ENOENT);
    assert( ret == -1);

    errno = 0;
    ret = stat("newFile", &s);
    assert( errno == 0);
    assert( ret == 0);
    
    
    close(fd);

    // 2nd must fail cause O_EXCL is set
    
    errno = 0;
    int fd2 = open("newFile", O_WRONLY | O_APPEND | O_CREAT | O_EXCL , 0644);
    assert( errno == EEXIST);
    assert( fd2 == -1);
    
 

    
    return 1;
}

static int doStatTests()
{
    return 1;
}

static int doCloseTests()
{
    errno = 0;
    int ret = close(100);
    assert(errno == EBADF);
    assert(ret == -1);
    
    errno = 0;
    ret = close(-1);
    assert(errno == EBADF);
    assert(ret == -1);
    
    
    return 1;
}


static int doDevNullTests()
{
    errno = 0;
    
    int fd =  open("/dev/null", O_RDWR);
    assert(fd >= 0);
    assert(errno == 0);
    
    const char t[] = "Some text";
    ssize_t retWrite = write(fd, t, sizeof(t) );
    assert( retWrite == sizeof(t));
    assert(errno == 0);
    
    char b[3] = {0};
    ssize_t retRead = read(fd, b, 3);

    assert( retRead == 0);    
    
    int ret = close(fd);
    assert(ret == 0);
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
    assert(doCloseTests());
    assert(doGetPidTests());
    assert(doTimeTests());
    assert(doOpenTests());
    assert(doOpenTests2());
    assert(doReadTests());

    assert(doWaitTests() );
    assert(doGetCwdTests() );
    
    assert(doDevNullTests());
    
    assert(doCreateAndStatFileTest() );
    
    assert(doUnlinkTests());
    
    // will change path to '/' so do this last !
    assert(doChdirTests());
    assert(doLsTests() );
    assert(doLsTests2() );
    
    assert(sleep(1) == 0);
    printf("SOFA : Everything is fine!\n");
    return 0;
}
