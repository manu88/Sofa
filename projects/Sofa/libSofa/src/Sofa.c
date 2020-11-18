#include <Sofa.h>
#include "syscalls.h"
#include <runtime.h>

int SofaSpawn(const char* path)
{
    if(!path)
    {
        return -1;
    }

    int ret = sc_spawn(TLSGet()->ep, TLSGet()->buffer, path);

    return ret;
}

pid_t SofaGetPid()
{
    return getProcessEnv()->pid;
}

pid_t SofaGetPPid()
{
    return sc_getppid(TLSGet()->ep);
}


pid_t SofaWaitPid(pid_t pid, int *wstatus, int options)
{
    return sc_wait(TLSGet()->ep, pid, wstatus, options);
}

pid_t SofaWait(int *wstatus)
{
    return SofaWaitPid(-1, wstatus, 0);
}

int SofaKill(pid_t pid, int sig)
{
    return sc_kill(TLSGet()->ep, pid, sig);
}

ssize_t SofaRead(char* data, size_t dataSize)
{
    return sc_read(TLSGet()->ep, data, dataSize, 0);
}

ssize_t SofaReadLine(char* data, size_t dataSize)
{
    return sc_read(TLSGet()->ep, data, dataSize, '\n');
}

void SofaDebug(SofaDebugCode code)
{
    sc_debug(TLSGet()->ep, code);
}

void exit(int code)
{
    sc_exit(getProcessEndpoint(), code);
    // no return
    assert(0);
}