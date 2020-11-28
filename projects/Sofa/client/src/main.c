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
        for (int i=0;i< 10;i++)
        {
            seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
            seL4_SetMR(0, i);
            seL4_SetMR(1, 10-i);        
            SFPrintf("Call service\n");
            seL4_Call((seL4_CPtr) capOrErr, info);
            SFPrintf("Service returned!\n");
        }
    }

    return 10 + SFGetPid();
}

