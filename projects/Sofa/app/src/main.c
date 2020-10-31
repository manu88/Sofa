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

    seL4_MessageInfo_t msg = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 1);
    printf("[App] test send\n");
    seL4_MessageInfo_t m =seL4_Call(cap, msg);
    printf("[App] got reply %lu\n", seL4_GetMR(0));
    while (1)
    {

    }
    
    return 0;
}

