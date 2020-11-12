#include <Sofa.h>
#include "syscalls.h"
#include "runtime.h"

int SofaSleep(int ms)
{
    return sc_sleep(getProcessEndpoint(), ms);
}

int SofaSleep2(int ep, int ms)
{
    return sc_sleep(ep, ms);
}