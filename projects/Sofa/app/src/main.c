#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>
#include <Spawn.h>
#include <Thread.h>
#include <sys_calls.h>

static int thread_run(seL4_CPtr ep)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_Call(ep, info);
    //printf("Hello from thread\n");
    //while(1);
}

int main(int argc, char *argv[])
{
    while (1)
    {
        /* code */
    }
    
    seL4_CPtr endpoint = (seL4_CPtr) atoi(argv[1]);

    int err = ProcessInit(endpoint);
    assert(err == 0);


    seL4_CPtr ep = vka_alloc_endpoint_leaky(&getProcessContext()->vka);

    helper_thread_t th;
    create_helper_thread(getProcessContext(), &th);
    printf("Thread created\n");

    start_helper(getProcessContext(), &th, thread_run, ep, 0, 0, 0);
    DoDebug(DebugCode_DumpScheduler);
    printf("call join\n");
    seL4_Word sender;
    seL4_MessageInfo_t info = seL4_Recv(ep, &sender);
    printf("Join returned\n");


    while (1)
    {
        seL4_Yield();
    }
    
    return 0;
}

