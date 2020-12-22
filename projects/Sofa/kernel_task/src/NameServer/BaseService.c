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


int BaseServiceCreate(BaseService*s, const char*name, BaseServiceCallbacks* ops)
{
    memset(s, 0, sizeof(BaseService));
    int error = 0;
    ServiceInit(&s->service, getKernelTaskProcess());
    s->service.name = name;
    s->callbacks = ops;

    ServiceCreateKernelTask(&s->service);
    NameServerRegister(&s->service);
}

static int mainBaseService(KThread* thread, void *arg)
{
    BaseService* service = (BaseService*) arg;
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