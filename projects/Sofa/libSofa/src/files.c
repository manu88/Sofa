#include "files.h"
#include "runtime.h"
#include <Sofa.h>
#include <stdarg.h>


seL4_CPtr vfsCap = 0;
char* vfsBuf = NULL;


int VFSClientInit()
{
    ssize_t capOrErr = SFGetService("VFS");

    if(capOrErr > 0)
    {
        vfsCap = capOrErr;

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, VFSRequest_Register);
        seL4_Call(vfsCap, info);
        vfsBuf = (char*) seL4_GetMR(1);
        return 0;
    }
    return -1;
}

void VFSDebug()
{
    if(vfsCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, VFSRequest_Debug);
    seL4_Send(vfsCap, info);
}

int VFSOpen(const char* path, int mode)
{
    if(vfsCap == 0)
    {
        SFPrintf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        SFPrintf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, VFSRequest_Open);
    seL4_SetMR(1, mode); // mode
    strcpy(vfsBuf, path);
    vfsBuf[strlen(path)] = 0;
    seL4_Call(vfsCap, info);
    int err = seL4_GetMR(1);
    int handle = seL4_GetMR(2);
    if(err != 0)
    {
        return -err;
    }
    return handle;
}

int VFSClose(int handle)
{
    if(vfsCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, VFSRequest_Close);
    seL4_SetMR(1, handle);
    seL4_Call(vfsCap, info);
    int err = seL4_GetMR(1);

    return -err;
}

int VFSSeek(int handle, size_t pos)
{
    if(vfsCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, VFSRequest_Seek);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, pos);
    seL4_Call(vfsCap, info);
    int err = seL4_GetMR(1);
    int offset = seL4_GetMR(2);

    if(err == 0)
    {
        return offset;
    }
    return -err;
}

ssize_t VFSWrite(int handle, const char* data, size_t size)
{
    if(vfsCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    memcpy(vfsBuf, data, size);

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, VFSRequest_Write);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, size);

    seL4_Call(vfsCap, info);

    int err = seL4_GetMR(1);
    int wSize = seL4_GetMR(2);
    if(err == 0)
    {
        return wSize;
    }

    return -err; 
}

ssize_t VFSRead(int handle, char* data, size_t size)
{
    if(vfsCap == 0)
    {
        Printf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        Printf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, VFSRequest_Read);
    seL4_SetMR(1, handle);
    seL4_SetMR(2, size);
    seL4_Call(vfsCap, info);
    int err = seL4_GetMR(1);
    int readSize = seL4_GetMR(2);
    if(err == 0)
    {
        memcpy(data, vfsBuf, readSize);
        return readSize;
    }

    return -err;
}


int Printf(const char *format, ...)
{
    assert(vfsCap);
    assert(vfsBuf);
    va_list args;

    va_start(args, format);
    int length = vsnprintf(vfsBuf, 4095, format, args);
    va_end(args);
    vfsBuf[length] = 0;

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, VFSRequest_Write);
    seL4_SetMR(1, 2);
    seL4_SetMR(2, length);

    seL4_Call(vfsCap, info);

    int err = seL4_GetMR(1);
    int readSize = seL4_GetMR(2);
    if(err == 0)
    {
        return readSize;
    }

    return -err;
}