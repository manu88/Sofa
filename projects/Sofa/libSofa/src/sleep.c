#include <Sofa.h>
#include "syscalls.h"
#include "runtime.h"

int SofaSleep(int ms)
{
    return sc_sleep(TLSGet()->ep, ms);
}

