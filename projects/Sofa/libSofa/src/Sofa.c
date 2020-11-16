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


pid_t SofaWaitPid(pid_t pid, int *wstatus, int options)
{
    return sc_wait(TLSGet()->ep, pid, wstatus, options);
}

pid_t SofaWait(int *wstatus)
{
    return SofaWaitPid(-1, wstatus, 0);
}

ssize_t SofaRead(char* data, size_t dataSize)
{
    return sc_read(TLSGet()->ep, data, dataSize);
}