#include <Sofa.h>
#include "syscalls.h"
#include <runtime.h>
#include <stdarg.h>

int SFSpawn(const char* path)
{
    if(!path)
    {
        return -1;
    }

    int ret = sc_spawn(TLSGet()->ep, TLSGet()->buffer, path);

    return ret;
}

pid_t SFGetPid()
{
    return getProcessEnv()->pid;
}

pid_t SFGetPPid()
{
    return sc_getppid(TLSGet()->ep);
}


pid_t SFWaitPid(pid_t pid, int *wstatus, int options)
{
    return sc_wait(TLSGet()->ep, pid, wstatus, options);
}

pid_t SFWait(int *wstatus)
{
    return SFWaitPid(-1, wstatus, 0);
}

int SFKill(pid_t pid, int sig)
{
    return sc_kill(TLSGet()->ep, pid, sig);
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