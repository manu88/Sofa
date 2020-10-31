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
        cap = test_GetCap();
        printf("Cap %lu\n", cap);
    }
    
    printf("[App] received cap\n");


    seL4_MessageInfo_t msg = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 1);
    seL4_Call(cap, msg);
    while (1)
    {

    }
    
    return 0;
}

