#include "net.h"
#include "runtime.h"
#include "files.h" // Printf
#include <Sofa.h>
#include <sel4/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

int NetBind(int familly, int protoc, int port)
{    
    if(netCap == 0)
    {
        Printf("[shell] Net client not registered (no cap)\n");
    }
    if(netBuf == NULL)
    {
        Printf("[shell] Net client not registered(no buff)\n");
    }


    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 4);
    seL4_SetMR(0, NetRequest_Bind);
    seL4_SetMR(1, familly);
    seL4_SetMR(2, protoc);
    seL4_SetMR(3, port);
    seL4_Call(netCap, info);

    return 0;
}


ssize_t NetWrite(int handle, const char* data, size_t size)
{
    if(netCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }

    if(netBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    memcpy(netBuf, data, size);

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, NetRequest_Write);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, size);

    seL4_Call(netCap, info);

    int err = seL4_GetMR(1);
    int wSize = seL4_GetMR(2);
    if(err == 0)
    {
        return wSize;
    }

    return -err; 

}

ssize_t NetRead(int handle, char* data, size_t size)
{
    if(netCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }

    if(netBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, NetRequest_Read);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, size);
    seL4_Call(netCap, info);
//    int err = seL4_GetMR(1);
    int readSize = seL4_GetMR(0);
    if(readSize)
    {
        memcpy(data, netBuf, readSize);
        data[readSize] = 0;

        return readSize;
    }

}
