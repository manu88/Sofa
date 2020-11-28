#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>



int main(int argc, char *argv[])
{   
    
    RuntimeInit2(argc, argv);
    SFPrintf("\n\n");
    SFPrintf("[%i] Client started\n", SFGetPid());

    ssize_t capOrErr = SFGetService("com.test.app");
    SFPrintf("SFGetService returned %li\n", capOrErr);

    if(capOrErr > 0)
    {
        while(1)
        {
            seL4_Word sender = 0;
            seL4_MessageInfo_t info = seL4_Recv(capOrErr, &sender);
            seL4_Word acc = seL4_GetMR(0);

            SFPrintf("[client %i] got %lu from %lu\n", SFGetPid(), acc, sender);
        }
    }

    return 10 + SFGetPid();
}

