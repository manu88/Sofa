#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>

static void* on_thread(void*args)
{
    printf("Hello thread %i\n", getProcessEnv()->pid);
    SFSleep(500);
    return (void*)12;
}

int main(int argc, char *argv[])
{   
    while (1)
    {
        /* code */
    }
    
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] started\n", SFGetPid());
    return 10 + SFGetPid();
    Thread th;
    ThreadInit(&th, on_thread, NULL);

    int retThread = 0;
    ThreadJoin(&th, (void**)&retThread);
    printf("[%i] thread returned %i\n", getProcessEnv()->pid, retThread);
    SFSleep(2000);
    //cleanup_helper(getProcessEnv(), &thread1);

    return 1;
}

