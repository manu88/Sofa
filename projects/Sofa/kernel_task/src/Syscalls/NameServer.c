#include <Sofa.h>
#include "SyscallTable.h"
#include "../NameServer.h"


void Syscall_RegisterService(Thread* caller, seL4_MessageInfo_t info)
{
    char *serviceName = strdup((const char *) caller->ipcBuffer);
    assert(serviceName);
    assert(serviceName);
    assert(strlen(serviceName));

    printf("Register Service name %s", serviceName);

    Service* newService = kmalloc(sizeof(Service));
    if(!newService)
    {
        seL4_SetMR(1, -ENOMEM);
        seL4_Reply(info);
        return;
    }

    ServiceInit(newService, caller->process);
    newService->name = serviceName;

    int err = NameServerRegister(newService);
    if(err)
    {
        free(newService);
        free(serviceName);
        seL4_SetMR(1, err);
        seL4_Reply(info);
        return;
    }

    KernelTaskContext* ctx = getKernelTaskContext();

    vka_object_t tcb_obj;
    vka_alloc_endpoint(&ctx->vka, &tcb_obj);
    cspacepath_t res;
    vka_cspace_make_path(&ctx->vka, tcb_obj.cptr, &res);
    seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->process->native, res, &ctx->vka);
    newService->endpoint = ret;

    seL4_SetMR(1, 0);
    seL4_SetMR(2, ret);
    seL4_Reply(info);
}