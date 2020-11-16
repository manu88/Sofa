#include <runtime.h>
#include <Sofa.h>

void sc_exit(seL4_CPtr endpoint, int code)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Exit);
    seL4_SetMR(1, code);
    seL4_Send(endpoint, info);
}

int sc_sleep(seL4_CPtr endpoint, int ms)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Sleep);
    seL4_SetMR(1, ms);
    info = seL4_Call(endpoint, info);
    return seL4_GetMR(1);
}


int sc_spawn(seL4_CPtr endpoint, uint8_t* ipcBuffer, const char* path)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Spawn);
    seL4_SetMR(1, strlen(path));

    memcpy(ipcBuffer, path, strlen(path));
    ipcBuffer[strlen(path)] = 0;
    info = seL4_Call(endpoint, info);

    return seL4_GetMR(1);    
}


int sc_wait(seL4_CPtr endpoint, pid_t pid, int *wstatus, int options)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SyscallID_Wait);
    seL4_SetMR(1, pid);
    seL4_SetMR(2, options);

    info = seL4_Call(endpoint, info);

    if (wstatus)
    {
        *wstatus = (int) seL4_GetMR(2);
    }

    return seL4_GetMR(1);
}


ssize_t sc_read(seL4_CPtr endpoint, char* data, size_t dataSize, char until)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SyscallID_Read);
    seL4_SetMR(1, dataSize);
    seL4_SetMR(2, (seL4_Word)until); 

    info = seL4_Call(endpoint, info);

    ssize_t readSize = seL4_GetMR(1);
    ssize_t effectiveSize = seL4_GetMR(1);
    if(readSize == -EAGAIN)
    {
        effectiveSize = dataSize;
    }
    memcpy(data, TLSGet()->buffer, effectiveSize );

    return readSize;
}