#include "runtime.h"
#include <Sofa.h>
#include <stdio.h>
#include <errno.h>
#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    
    RuntimeInit2(argc, argv);
    VFSClientInit();

    SFPrintf("App2 started\n");

    char* ptr = mmap(NULL, 4096, 0, 0,-1, 0);
    int ret = munmap(ptr, 4096);
    SFPrintf("munmap ret %i\n", ret);

    int fd0 = open("/fake/file1", O_RDONLY);
    int fd1 = open("/fake/cons", O_WRONLY);
    int fd2 = open("/fake/cons", O_WRONLY);

    //write(2, "Some msg test\n", strlen("Some msg test\n"));
    
    SFPrintf("fd0=%i %i %i\n", fd0, fd1, fd2);

    SFPrintf("--> start dlopen\n");
    void *t = dlopen("file", RTLD_NOW);
    SFPrintf("dlopen returned %p err=%i\n", t, errno);

    return 1;
}

