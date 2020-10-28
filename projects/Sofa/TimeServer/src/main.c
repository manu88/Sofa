#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>


int main(int argc, char *argv[])
{
    int ret = ProcessInit((void*) atoi(argv[1]));
    assert(ret == 0);

    printf("TimeServer started pid is %i ppid is %i\n", getpid(), getppid());
    
    return 0;
}

