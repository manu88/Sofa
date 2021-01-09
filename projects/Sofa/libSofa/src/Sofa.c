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
#include <Sofa.h>
#include "syscalls.h"
#include <runtime.h>
#include <stdarg.h>


pid_t SFGetPid()
{
    return getProcessEnv()->pid;
}

pid_t SFGetPPid()
{
    return sc_getppid(TLSGet()->ep);
}





ssize_t SFRead(char* data, size_t dataSize)
{
    return sc_read(TLSGet()->ep, data, dataSize, 0);
}

ssize_t SFWrite(const char* data, size_t dataSize)
{
    return sc_write(TLSGet()->ep, data, dataSize);
}

ssize_t SFReadLine(char* data, size_t dataSize)
{
    return sc_read(TLSGet()->ep, data, dataSize, '\n');
}

void SFDebug(SofaDebugCode code)
{
    sc_debug(TLSGet()->ep, code);
}

void exit(int code)
{
    sc_exit(getProcessEndpoint(), code);
    // no return
    assert(0);
}

int SFPrintf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    int length = vsnprintf(TLSGet()->buffer, 4096, format, args);
    va_end(args);
    TLSGet()->buffer[length] = 0;


    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Write);
    seL4_SetMR(1, length);

    seL4_Send(TLSGet()->ep, info);
    return length;
}

ssize_t SFGetService(const char* name)
{
    if(!name)
    {
        return -EINVAL;
    }
    if(strlen(name) == 0)
    {
        return -EINVAL;
    }
    int err = 0;
    seL4_CPtr cap = sc_getservice(TLSGet()->ep, name, &err);
    if(err != 0)
    {
        return err;
    }
    return cap;
}

ssize_t SFRegisterService(const char* name)
{
    if(!name)
    {
        return -EINVAL;
    }
    if(strlen(name) == 0)
    {
        return -EINVAL;
    }
    int err = 0;
    seL4_CPtr cap = sc_regservice(TLSGet()->ep, name, &err);
    if(err != 0)
    {
        return err;
    }
    return cap;
}

int SFShutdown(void)
{
    return sc_reboot(TLSGet()->ep, RebootMode_Shutdown);
}


long SFMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return sc_mmap(TLSGet()->ep, addr, length, prot, flags, fd, offset);
}

long SFMunmap(void* addr, size_t length)
{
    return sc_munmap(TLSGet()->ep, addr, length);
}


long SFShareMem(void* addr, seL4_Word with, seL4_CapRights_t rights)
{
    return sc_sharemem(TLSGet()->ep, addr, with, rights.words[0]);
}