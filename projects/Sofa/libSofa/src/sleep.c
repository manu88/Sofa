#include <Sofa.h>
#include "syscalls.h"
#include "runtime.h"

int SofaSleep(int ms)
{
    if(ms == 0)
    {
        return 0;
    }
    return sc_sleep(TLSGet()->ep, ms);
}

