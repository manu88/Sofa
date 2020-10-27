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

    printf("Init started\n");

    pid_t timeServerPID = 0;
    ret = posix_spawnp(&timeServerPID, "TimeServer", NULL, NULL, NULL, NULL);
    printf("Spawn returned %i, pid is %i\n", ret, timeServerPID);
    while (1)
    {

    }
    
    return 0;
}

