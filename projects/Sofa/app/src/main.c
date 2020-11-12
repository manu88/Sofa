#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"


static int on_thread1(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    printf("Hello thread1\n");
    return 0;
}



int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    printf("[%i] started\n", getProcessEnv()->pid);

    helper_thread_t thread1;
    create_helper_thread(getProcessEnv(), &thread1);
    start_helper(getProcessEnv(), &thread1, on_thread1, 0, 0, 0, 0);

    printf("[%i] thread returned\n", getProcessEnv()->pid);

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 42);
    seL4_Send(getProcessEndpoint(), info);

}

