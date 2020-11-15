#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>

static void* on_thread(void*args)
{
    printf("Hello thread %i\n", getProcessEnv()->pid);
    SofaSleep(2000);
    return (void*)12;
}

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] started\n", getProcessEnv()->pid);

    Thread th;
    ThreadInit(&th, on_thread, NULL);

    int retThread = 0;
    ThreadJoin(&th, (void**)&retThread);
    printf("[%i] thread returned %i\n", getProcessEnv()->pid, retThread);
    SofaSleep(2000);
    //cleanup_helper(getProcessEnv(), &thread1);

    return 1;
}

