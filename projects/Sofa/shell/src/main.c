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
    
  
    printf("Shell started parent pid is %i\n", getppid());


    while (1)
    {

    }
    
    return 0;
}

