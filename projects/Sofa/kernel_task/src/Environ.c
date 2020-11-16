#include "Environ.h"


static KernelTaskContext _ctx;

KernelTaskContext* getKernelTaskContext(void)
{
    return &_ctx;
}