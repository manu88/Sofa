#include <Sofa.h>
#include "syscalls.h"
#include <runtime.h>

int SofaSpawn(const char* path)
{
    if(!path)
    {
        return -1;
    }

    int ret = sc_spawn(TLSGet()->ep, path);
}