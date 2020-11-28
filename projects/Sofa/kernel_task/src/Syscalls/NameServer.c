#include <Sofa.h>
#include "SyscallTable.h"
#include "../NameServer.h"
#include "../utils.h"

void Syscall_GetService(Thread* caller, seL4_MessageInfo_t info)
{
    char *serviceName = /*strdup((const char *)*/ caller->ipcBuffer;
    assert(serviceName);
    assert(strlen(serviceName));

    printf("Get Service name %s\n", serviceName);

    Service* service = NameServerFind(serviceName);
    if(service == NULL)
    {
        printf("Not found\n");
        seL4_SetMR(1, -ENOENT);
        seL4_Reply(info);
        return;
    }
    assert(service->endpoint != seL4_CapNull);

    KernelTaskContext* env = getKernelTaskContext();

    seL4_CPtr capMint = get_free_slot(&env->vka);
    int err = cnode_mint(&env->vka, service->baseEndpoint, capMint, seL4_AllRights, caller);
    printf("MINT value %X\n", (seL4_Word) caller);
    assert(err == 0);
    cspacepath_t path;
    vka_cspace_make_path(&env->vka, capMint, &path);

    seL4_CPtr cap = sel4utils_move_cap_to_process(&caller->process->native, path, &env->vka);
    printf("Create a client cap : %lu\n", cap);

    seL4_SetMR(1, 0);
    seL4_SetMR(2, cap);
    seL4_Reply(info);
}


void Syscall_RegisterService(Thread* caller, seL4_MessageInfo_t info)
{
    char *serviceName = strdup((const char *) caller->ipcBuffer);
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

    seL4_CPtr ret = sel4utils_copy_cap_to_process(&caller->process->native, &ctx->vka, res.capPtr);
    //seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->process->native, res, &ctx->vka);
    newService->endpoint = ret;
    newService->baseEndpoint = res.capPtr;
    seL4_SetMR(1, 0);
    seL4_SetMR(2, ret);
    seL4_Reply(info);
}