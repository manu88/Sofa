#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"


static int on_thread1(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    printf("Hello thread %i\n", getProcessEnv()->pid);
    return 0;
}

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] started\n", getProcessEnv()->pid);

    helper_thread_t thread1;
    create_helper_thread(getProcessEnv(), &thread1);
    start_helper(getProcessEnv(), &thread1, on_thread1, 0, 0, 0, 0);
    wait_for_helper(&thread1);
    printf("[%i] thread returned\n", getProcessEnv()->pid);

    return 1;
}

