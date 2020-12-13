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


int NetSocket(int domain, int type, int protocol)
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
    seL4_SetMR(0, NetRequest_Socket);
    seL4_SetMR(1, domain);
    seL4_SetMR(2, type);
    seL4_SetMR(3, protocol);
    seL4_Call(netCap, info);

    int err = seL4_GetMR(1);
    int handle = seL4_GetMR(2);
    if(err != 0)
    {
        return -err;
    }
    return handle;
}

int NetBind(int handle, const struct sockaddr *addr, socklen_t addrlen)
{    
    if(netCap == 0)
    {
        Printf("[shell] Net client not registered (no cap)\n");
    }
    if(netBuf == NULL)
    {
        Printf("[shell] Net client not registered(no buff)\n");
    }


    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 5);
    seL4_SetMR(0, NetRequest_Bind);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, addrlen);
    memcpy(netBuf, addr, addrlen);
/*    
    seL4_SetMR(2, familly);
    seL4_SetMR(3, protoc);
    seL4_SetMR(4, port);
*/  
    seL4_Call(netCap, info);

    return seL4_GetMR(1);
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
ssize_t NetSendTo(int handle, const void *buf, size_t bufLen, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    if(netCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }

    if(netBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 4);
    seL4_SetMR(0, NetRequest_SendTo);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, bufLen);
    seL4_SetMR(3, addrlen);
    memcpy(netBuf, dest_addr, addrlen);
    memcpy(netBuf + addrlen, buf, bufLen);
    info = seL4_Call(netCap, info);

    int sentLen = seL4_GetMR(2);
    int err = seL4_GetMR(3);

    if(err == 0)
    {
        return sentLen;
    }
    return err;
}

ssize_t NetRecvFrom(int handle, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
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
    seL4_SetMR(0, NetRequest_RecvFrom);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, len);
    seL4_Call(netCap, info);

    int readSize = seL4_GetMR(1);
    size_t addrSize = seL4_GetMR(2);
    if(readSize)
    {
        const char* addr = netBuf;
        memcpy(src_addr, addr, addrSize);
        *addrlen = addrSize;

        const char* dataPos = netBuf + addrSize;

        memcpy(buf, dataPos, readSize);
        //((char*)buf)[readSize] = 0;

        return readSize;
    }

    return -1;

}
