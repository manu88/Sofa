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
        SFPrintf("[APP] error unable to register service. error %li\n", servOrErr);
        return EXIT_FAILURE;
    }

    seL4_Word acc = 0;
    while(1)
    {
        seL4_CPtr serv = servOrErr;
        seL4_Word sender;
        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
        seL4_SetMR(0, acc++);
        seL4_Send(serv, info);
        SFSleep(1000);
    }

    return 1;
}

