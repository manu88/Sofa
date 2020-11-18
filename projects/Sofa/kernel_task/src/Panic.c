#include "Panic.h"
#include <stdio.h>
#include <assert.h>

void Panic(const char* reason)
{
    printf("PANIC : %s\n", reason);
    assert(0);
}