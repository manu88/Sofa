#include "NameServer.h"

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