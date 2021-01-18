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
#include "BaseService.h"
#include "Environ.h"
#include "Log.h"
#include "Process.h"
#include <utils/uthash.h>


int BaseServiceCreate(BaseService*s, const char*name, BaseServiceCallbacks* ops)
{
    memset(s, 0, sizeof(BaseService));
    ServiceInit(&s->service, getKernelTaskProcess());
    s->service.name = name;
    s->callbacks = ops;

    ServiceCreateKernelTask(&s->service);
    NameServerRegister(&s->service);

    return 0;
}

static int mainBaseService(KThread* thread, void *arg)
{
    BaseService* service = (BaseService*) arg;

    if(service->callbacks->onServiceStart)
    {
        service->callbacks->onServiceStart(service);
    }
    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(service->service.baseEndpoint, &sender);
        ThreadBase* caller =(ThreadBase*) sender;
        assert(caller->process);

        if (caller->process == getKernelTaskProcess())
        {
            service->callbacks->onSystemMsg(service, msg);
        }
        else
        {
            service->callbacks->onClientMsg(service, caller, msg);
        }
    }    
}

int BaseServiceStart(BaseService*s)
{
    KThreadInit(&s->thread);
    s->thread.mainFunction = mainBaseService;
    s->thread.name = s->service.name;
    int error = KThreadRun(&s->thread, 254, s);

    return error;
}


int BaseServiceCreateClientContext(BaseService* service, ThreadBase* sender, ServiceClient* client, size_t numPages)
{

    KernelTaskContext* env = getKernelTaskContext();
    char* buff = vspace_new_pages(&env->vspace, seL4_ReadWrite, numPages, PAGE_BITS_4K);
    assert(buff);
    void* buffShared = vspace_share_mem(&env->vspace,
                                        &sender->process->native.vspace,
                                        buff,
                                        numPages,
                                        PAGE_BITS_4K,
                                        seL4_ReadWrite,
                                        1
                                        );
    assert(buffShared);
    client->caller = sender;
    client->buff = buff;
    client->buffClientAddr = buffShared;
    client->service = &service->service;

    HASH_ADD_PTR(service->_clients, caller, client);

    ThreadBaseAddServiceClient(sender, client);

    return 0;
}

ServiceClient* BaseServiceGetClient(BaseService*s, ThreadBase* caller)
{
    ServiceClient* clt = NULL;
    HASH_FIND_PTR(s->_clients, &caller, clt );
    return clt;
}