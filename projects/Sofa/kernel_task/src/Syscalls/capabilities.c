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
#include "Process.h"


static void RequestNotification(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* ctx = getKernelTaskContext();

    vka_object_t notifObj = {0};
    seL4_Word err = vka_alloc_notification(&ctx->vka, &notifObj);
    if(err == 0)
    {
        cspacepath_t res;
        vka_cspace_make_path(&getKernelTaskContext()->vka, notifObj.cptr, &res);
        seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &ctx->vka);

        err = ret;
    }
    seL4_SetMR(1, err);
    seL4_Reply(info);
}

static void RequestEndpoint(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* ctx = getKernelTaskContext();

    vka_object_t notifObj = {0};
    seL4_Word err = vka_alloc_endpoint(&ctx->vka, &notifObj);
    if(err == 0)
    {
        cspacepath_t res;
        vka_cspace_make_path(&getKernelTaskContext()->vka, notifObj.cptr, &res);
        seL4_CPtr ret = sel4utils_move_cap_to_process(&caller->_base.process->native, res, &ctx->vka);

        err = ret;
    }
    seL4_SetMR(1, err);
    seL4_Reply(info);
}

void Syscall_RequestCap(Thread* caller, seL4_MessageInfo_t info)
{
    CapRequest req = (CapRequest) seL4_GetMR(1);

    switch (req)
    {
    case CapRequest_Notification:
        RequestNotification(caller, info);
        break;
    case CapRequest_Endpoint:
        RequestEndpoint(caller, info);
        break;
    default:
        KLOG_DEBUG("Syscall_RequestCap: request for unknown %i\n", req);
        break;
    } 
}