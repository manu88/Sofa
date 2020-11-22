#include "Environ.h"
#include <sel4platsupport/io.h>
#include <sel4platsupport/arch/io.h>
#include <sel4utils/page_dma.h>


static KernelTaskContext _ctx;

KernelTaskContext* getKernelTaskContext(void)
{
    return &_ctx;
}

int IOInit()
{
    KernelTaskContext* env = getKernelTaskContext();
    int error = sel4platsupport_new_io_ops(&env->vspace, &env->vka, &env->simple, &env->ops);
    assert(error == 0);

    error = sel4platsupport_get_io_port_ops(&env->ops.io_port_ops, &env->simple, &env->vka);
    assert(error == 0);

    error = sel4utils_new_page_dma_alloc(&env->vka, &env->vspace, &env->ops.dma_manager);
    assert(error == 0);

}