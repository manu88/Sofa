/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <runtime.h>
#include <Sofa.h>
#include "syscalls.h"

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


ssize_t sc_write(seL4_CPtr endpoint, const char* data, size_t dataSize)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Write);
    seL4_SetMR(1, dataSize);

    size_t effectiveSize = dataSize; 
    memcpy(TLSGet()->buffer, data, effectiveSize);
    TLSGet()->buffer[effectiveSize] = 0;

    seL4_Send(endpoint, info);
    return effectiveSize;

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
    else if(readSize < 0)
    {
        return readSize;
    }
    memcpy(data, TLSGet()->buffer, effectiveSize );

    return readSize;
}

void sc_debug(seL4_CPtr endpoint, SofaDebugCode code)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Debug);
    seL4_SetMR(1, code);

    seL4_Send(endpoint, info);
}




pid_t sc_getppid(seL4_CPtr endpoint)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_PPID);

    seL4_Call(endpoint, info);
    return seL4_GetMR(1);
}

seL4_CPtr sc_getservice(seL4_CPtr endpoint, const char* serviceName, int *err)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SofaSysCall_GetService);
    // 1 status
    // 2 endpoint

    const size_t nameSize = strlen(serviceName);
    memcpy(TLSGet()->buffer, serviceName, nameSize);
    TLSGet()->buffer[nameSize] = 0;

    seL4_Call(endpoint, info);

    if(err)
    {
        *err = seL4_GetMR(1); 
    }
    if(seL4_GetMR(1) == 0)
    {
        return seL4_GetMR(2);
    }
    return seL4_CapNull;


}

seL4_CPtr sc_regservice(seL4_CPtr endpoint, const char* serviceName, int *err)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SofaSysCall_RegisterService);
    // 1 status
    // 2 endpoint

    const size_t nameSize = strlen(serviceName);
    memcpy(TLSGet()->buffer, serviceName, nameSize);
    TLSGet()->buffer[nameSize] = 0;

    seL4_Call(endpoint, info);

    if(err)
    {
        *err = seL4_GetMR(1); 
    }
    if(seL4_GetMR(1) == 0)
    {
        return seL4_GetMR(2);
    }
    return seL4_CapNull;
}


int sc_reboot(seL4_CPtr endoint, int code)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SofaSysCall_Reboot);
    seL4_SetMR(1, code);
    seL4_Call(endoint, info);

    return seL4_GetMR(1);
}

long sc_mmap(seL4_CPtr endpoint, void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 7);
    seL4_SetMR(0, SyscallID_mmap);
    seL4_SetMR(1, addr);
    seL4_SetMR(2, length);
    seL4_SetMR(3, prot);
    seL4_SetMR(4, flags);
    seL4_SetMR(5, fd);
    seL4_SetMR(6, offset);
    seL4_Call(endpoint, info);

    return seL4_GetMR(1);
}

long sc_munmap(seL4_CPtr endpoint, void* addr, size_t length)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SyscallID_munmap);
    seL4_SetMR(1, addr);
    seL4_SetMR(2, length);
    seL4_Call(endpoint, info);

    return seL4_GetMR(1);
}



int sc_newThread(seL4_CPtr endpoint, ThreadConfig* conf)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 8);
    seL4_SetMR(0, SyscallID_ThreadNew);

    /*
    MR 1 status : 0 ok
       2 tcb
       3 ep
       4 ipcbuf
       5 ipcbufAddr
       6 stacktop
       7 shared buffer
    */
    seL4_Call(endpoint, info);

    if(seL4_GetMR(1) != 0)
    {
        return seL4_GetMR(1);
    }

    conf->tcb = seL4_GetMR(2);
    conf->ep = seL4_GetMR(3);
    conf->ipcBuf = seL4_GetMR(4);
    conf->ipcBufAddr = seL4_GetMR(5);
    conf->stackTop = (void*) seL4_GetMR(6);
    conf->sofaIPC = (void*) seL4_GetMR(7);

    return 0;
}