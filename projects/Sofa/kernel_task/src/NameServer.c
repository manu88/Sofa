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
    KernelTaskContext* ctx = getKernelTaskContext();
    int error = 0;
    vka_object_t ep = {0};
    error = vka_alloc_endpoint(&ctx->vka, &ep);
    assert(error == 0);
    s->baseEndpoint = ep.cptr;

    vka_object_t ep2 = {0};
    error = vka_alloc_endpoint(&ctx->vka, &ep2);
    assert(error == 0);
    
    s->kernTaskEp = get_free_slot(&ctx->vka);
    cnode_mint(&ctx->vka, ep.cptr, s->kernTaskEp, seL4_CanWrite, (seL4_Word)&_mainThread);

    return 0;
}