#include "runtime.h"
#include <Sofa.h>
#include <stdarg.h>
#include "dk.h"


seL4_CPtr dkCap = 0;
char* dkBuf = NULL;


int DKClientInit()
{
    if(dkCap && dkBuf)
    {
        return 0;
    }

    ssize_t capOrErr = SFGetService(DeviceKitServiceName);

    if(capOrErr > 0)
    {
        dkCap = capOrErr;

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, DKRequest_Register);
        seL4_Call(dkCap, info);
        dkBuf = (char*) seL4_GetMR(1);
        return 0;
    }
    return -1;
}

int DKClientEnum(void)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, DKRequest_List);
    seL4_Send(dkCap, info);

    return 0;
}
