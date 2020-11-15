#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include "Sofa.h"

void sc_exit(seL4_CPtr endpoint, int code);

static void
thread_init_tls(helper_thread_t *thread)
{
 //   thread->info.ipc_word = seL4_GetUserData();
 //   assert(thread->info.ipc_word != 0);
    seL4_SetUserData((seL4_Word)thread);
}

static void _sendThreadExit(seL4_CPtr ep)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, SyscallID_ThreadExit);

    seL4_Send(ep, info);
}

static int on_thread1(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    printf("Hello thread %i\n", getProcessEnv()->pid);

    
//    _sendThreadExit(ep);
    
    return 0;
}

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] started\n", getProcessEnv()->pid);
    SofaSleep(2000);
    return 0;
    seL4_CPtr ep = 0;// getNewThreadEndpoint();
    helper_thread_t thread1;
    create_helper_thread(getProcessEnv(), &thread1);

    start_helper(getProcessEnv(), &thread1, on_thread1, ep, 0, 0, 0);


    wait_for_helper(&thread1);
    printf("[%i] thread returned\n", getProcessEnv()->pid);
    //cleanup_helper(getProcessEnv(), &thread1);

    return 1;
}

