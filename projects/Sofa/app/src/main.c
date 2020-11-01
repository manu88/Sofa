#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>
#include <Spawn.h>

int main(int argc, char *argv[])
{
    int ret = ProcessInit(atoi(argv[1]));
    assert(ret == 0);
    
  
    printf("App started parent pid is %i\n", getppid());


    seL4_CPtr cap = 0;
    while (cap ==  0)
    {
        cap = getIPCService("TimeServer.main");
    }
    
    printf("[App] received cap\n");


    tempSetTimeServerEP(cap);
    struct timespec tm;
    tm.tv_sec = 2;
    tm.tv_nsec = 0;

    nanosleep(&tm, NULL);

    while (1)
    {

    }
    
    return 0;
}

