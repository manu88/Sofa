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

    vka_t *mainVKA = getMainVKA();

    seL4_CPtr capMint = get_free_slot(mainVKA);
    int err = cnode_mint(mainVKA, service->baseEndpoint, capMint, seL4_AllRights, (seL4_Word) caller);
    assert(err == 0);
    cspacepath_t path;
    vka_cspace_make_path(mainVKA, capMint, &path);

    seL4_CPtr cap = sel4utils_move_cap_to_process(&caller->_base.process->native, path, mainVKA);

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
    vka_t *mainVKA = getMainVKA();
    KLOG_DEBUG("Create Kernel task endpoint\n");
    vka_object_t tcb_obj;
    vka_alloc_endpoint(mainVKA, &tcb_obj);
    cspacepath_t res;
    vka_cspace_make_path(mainVKA, tcb_obj.cptr, &res);

// create a minted enpoint for the thread
    KLOG_DEBUG("Create Process task endpoint\n");

    vka_object_t tcb_obj2;
    vka_alloc_endpoint(mainVKA, &tcb_obj2);
    cspacepath_t res2;
    vka_cspace_make_path(mainVKA, tcb_obj2.cptr, &res2);


    KLOG_DEBUG("Copy cap to process\n");
    seL4_CPtr ret = sel4utils_copy_cap_to_process(&caller->_base.process->native, mainVKA, res.capPtr);
    newService->endpoint = ret;
    newService->baseEndpoint = res.capPtr;
    newService->kernTaskEp = res2.capPtr;
    seL4_SetMR(1, 0);
    seL4_SetMR(2, ret);
    seL4_Reply(info);

}