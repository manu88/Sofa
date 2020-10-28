#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>
#include <Spawn.h>

int main(int argc, char *argv[])
{
    int ret = ProcessInit((void*) atoi(argv[1]));
    assert(ret == 0);
    
    
    if(getpid() != 1)
    {
        printf("Init is not PID 1, will stop!\n");
        return 0;
    }


    printf("Init started parent pid is %i\n", getppid());

    pid_t timeServerPID = 0;
    ret = posix_spawnp(&timeServerPID, "TimeServer", NULL, NULL, NULL, NULL);
    printf("Spawn returned %i, pid is %i\n", ret, timeServerPID);

    while (1)
    {
        int status = 0;
        pid_t pid = wait(&status);
        printf("Wait returned pid %i status %i\n", pid, status);
    }
    
    return 0;
}

