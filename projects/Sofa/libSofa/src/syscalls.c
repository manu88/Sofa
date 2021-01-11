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

uint64_t sc_gettime(seL4_CPtr endpoint)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SyscallID_GetTime);
    info = seL4_Call(endpoint, info);

    union
    {
        uint64_t v;
        uint32_t s[2];
    } value;
    value.s[0] = seL4_GetMR(1);
    value.s[1] = seL4_GetMR(2);    
    return value.v;
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


long sc_sharemem(seL4_CPtr endpoint, void* addr, seL4_Word with, uint64_t rights)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 4);
    seL4_SetMR(0, SyscallID_ShareMem);
    seL4_SetMR(1, addr);
    seL4_SetMR(2, with);
    seL4_SetMR(3, rights);
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

long sc_caprequest(seL4_CPtr endpoint, CapRequest type)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0, SyscallID_RequestCap);
    seL4_SetMR(1, type);
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