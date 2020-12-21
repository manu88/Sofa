#include "Panic.h"
#include "Log.h"
#include <stdio.h>
#include <assert.h>

void Panic(const char* reason)
{
    KLOG_ERROR("PANIC : %s\n", reason);
    assert(0);
}