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
    
    RuntimeInit2(argc, argv);
    SFPrintf("\n\n");
    SFPrintf("[%i] started\n", SFGetPid());


    ssize_t servOrErr = SFRegisterService("com.test.app");
    if(servOrErr <= 0)
    {
        SFPrintf("[APP] error unable to register service. error %i\n", servOrErr);
        return EXIT_FAILURE;
    }

    while(1)
    {
        seL4_CPtr serv = servOrErr;
        seL4_Word sender;
        seL4_MessageInfo_t info = seL4_Recv(serv, &sender);
        seL4_Word m0 = seL4_GetMR(0);
        seL4_Word m1 = seL4_GetMR(1);
        SFPrintf("App received something from %lX len %li\n", sender, seL4_MessageInfo_get_length(info));

        seL4_Reply(info);
        SFPrintf("MR0 %li\n", m0);
        SFPrintf("MR1 %li\n", m1);

    }
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

