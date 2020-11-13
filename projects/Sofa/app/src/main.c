#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include "Sofa.h"

void sc_exit(seL4_CPtr endpoint, int code);

static void _sendThreadExit(seL4_CPtr ep)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, SyscallID_ThreadExit);

    seL4_Send(ep, info);
}

static int on_thread1(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    printf("Hello thread %i\n", getProcessEnv()->pid);

    //while(1)
    {
        int ret = SofaSleep2(ep, 1000);
        printf("Thread Sleep returned %i\n", ret);
        sc_exit(ep, 0);
    }
    
    _sendThreadExit(ep);
    
    return 0;
}

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] started\n", getProcessEnv()->pid);

    seL4_CPtr ep =  getNewThreadEndpoint();
    printf("Got a new thread enpoint\n");
    helper_thread_t thread1;
    create_helper_thread(getProcessEnv(), &thread1);

    start_helper(getProcessEnv(), &thread1, on_thread1, ep, 0, 0, 0);

    SofaSleep(4000);
    return 0;

    wait_for_helper(&thread1);
    printf("[%i] thread returned\n", getProcessEnv()->pid);

    return 1;
}

