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
#include "NameServer.h"
#include "ProcessList.h"
#include "KThread.h"
#include "utils.h"

static Service* _services = NULL;


Service* NameServerGetServices()
{
    return _services;
}

int NameServerInit()
{
    return 0;
}


int NameServerRegister(Service* s)
{
    if(NameServerFind(s->name))
    {
        return -EEXIST;
    }
    HASH_ADD_KEYPTR(hh, _services, s->name, strlen(s->name), s);    
    return 0;
}

int NameServerUnregister(Service* s)
{
    HASH_DEL(_services, s);
}

Service* NameServerFind(const char *name)
{
    Service* s = NULL;
    HASH_FIND_STR(_services, name, s);
    return s;
}

extern KThread _mainThread;

int ServiceCreateKernelTask(Service* s)
{
    vka_t *mainVKA = getMainVKA();
    int error = 0;
    vka_object_t ep = {0};
    error = vka_alloc_endpoint(mainVKA, &ep);
    assert(error == 0);
    s->baseEndpoint = ep.cptr;

    vka_object_t ep2 = {0};
    error = vka_alloc_endpoint(mainVKA, &ep2);
    assert(error == 0);
    
    s->kernTaskEp = get_free_slot(mainVKA);
    cnode_mint(mainVKA, ep.cptr, s->kernTaskEp, seL4_CanWrite, (seL4_Word)&_mainThread);

    return 0;
}