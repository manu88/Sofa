#include "net.h"
#include "runtime.h"
#include <Sofa.h>
#include <sel4/types.h>


seL4_CPtr netCap = 0;
char* netBuf = NULL;

int NetInit()
{
    ssize_t capOrErr = SFGetService("NET");

    if(capOrErr > 0)
    {
        netCap = capOrErr;

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, NetRequest_Register);
        seL4_Call(netCap, info);
        netBuf = (char*) seL4_GetMR(1);
        return 0;
    }
    return -1;

}