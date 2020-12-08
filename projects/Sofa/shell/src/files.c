#include "files.h"
#include "runtime.h"
#include <Sofa.h>


seL4_CPtr vfsCap = 0;
char* vfsBuf = NULL;


int VFSClientInit()
{
    ssize_t capOrErr = SFGetService("VFS");
    SFPrintf("SFGetService returned %li\n", capOrErr);

    if(capOrErr > 0)
    {
        vfsCap = capOrErr;

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, VFSRequest_Register);
        seL4_Call(vfsCap, info);
        vfsBuf = (char*) seL4_GetMR(1);
        SFPrintf("VFS client ok\n");
        return 0;
    }
    return -1;
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
    seL4_SetMR(1, 1); // mode
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
        SFPrintf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        SFPrintf("[shell] VFS client not registered(no buff)\n");
    }

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, VFSRequest_Close);
    seL4_SetMR(1, handle);
    seL4_Call(vfsCap, info);
    int err = seL4_GetMR(1);

    return -err;
}

ssize_t VFSRead(int handle, char* data, size_t size)
{
    if(vfsCap == 0)
    {
        SFPrintf("[shell] VFS client not registered (no cap)\n");
    }
    if(vfsBuf == NULL)
    {
        SFPrintf("[shell] VFS client not registered(no buff)\n");
    }
    SFPrintf("Read request handle=%i, size=%i\n", handle, size);

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
    else
    {
        SFPrintf("got read response err=%i size=%i\n", err, readSize);        
    }

    return -1;
}