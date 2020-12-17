#include "runtime.h"
#include <Sofa.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
void* start(void* a)
{
    while(1);
}

int main(int argc, char *argv[])
{
    
    RuntimeInit2(argc, argv);
    VFSClientInit();

    SFPrintf("App2 started\n");

    return 0;
}

