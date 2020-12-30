/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <Sofa.h>
#include "SyscallTable.h"
#include "NameServer.h"
#include "utils.h"
#include <vka/capops.h>

void Syscall_GetService(Thread* caller, seL4_MessageInfo_t info)
{
    char *serviceName = /*strdup((const char *)*/ caller->ipcBuffer;
    assert(serviceName);
    assert(strlen(serviceName));

    Service* service = NameServerFind(serviceName);
    if(service == NULL)
    {
        seL4_SetMR(1, -ENOENT);
        seL4_Reply(info);
        return;
    }
    assert(service->baseEndpoint != seL4_CapNull);

    KernelTaskContext* env = getKernelTaskContext();

    seL4_CPtr capMint = get_free_slot(&env->vka);
    int err = cnode_mint(&env->vka, service->baseEndpoint, capMint, seL4_AllRights, (seL4_Word) caller);
    assert(err == 0);
    cspacepath_t path;
    vka_cspace_make_path(&env->vka, capMint, &path);

    seL4_CPtr cap = sel4utils_move_cap_to_process(&caller->_base.process->native, path, &env->vka);

    seL4_SetMR(1, 0);
    seL4_SetMR(2, cap);
    seL4_Reply(info);
}


void Syscall_RegisterService(Thread* caller, seL4_MessageInfo_t info)
{
    char *serviceName = strdup((const char *) caller->ipcBuffer);
    assert(serviceName);
    assert(strlen(serviceName));

    KLOG_INFO("Register Service name %s", serviceName);

    Service* newService = kmalloc(sizeof(Service));
    if(!newService)
    {
        seL4_SetMR(1, -ENOMEM);
        seL4_Reply(info);
        return;
    }

    ServiceInit(newService, caller->_base.process);
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

// create a minted enpoint for the thread

    vka_object_t tcb_obj2;
    vka_alloc_endpoint(&ctx->vka, &tcb_obj2);
    cspacepath_t res2;
    vka_cspace_make_path(&ctx->vka, tcb_obj2.cptr, &res2);

    //assert(vka_cnode_mint(&res2, &res, seL4_AllRights, 0) == 0);


    seL4_CPtr ret = sel4utils_copy_cap_to_process(&caller->_base.process->native, &ctx->vka, res.capPtr);
    newService->endpoint = ret;
    newService->baseEndpoint = res.capPtr;
    newService->kernTaskEp = res2.capPtr;
    seL4_SetMR(1, 0);
    seL4_SetMR(2, ret);
    seL4_Reply(info);

}