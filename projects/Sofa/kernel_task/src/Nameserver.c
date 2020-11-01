#include "Nameserver.h"


static IPCService* _services = NULL;

IPCService* NameServer_GetServices()
{
    return _services;
}

int NameServer_AddService(IPCService* s)
{
    HASH_ADD_KEYPTR(hh, _services, s->name, strlen(s->name), s);
    return 0;
}

int NameServer_RemoveService(IPCService* s)
{
    HASH_DEL(_services, s);
}

IPCService* NameServer_FindService(const char* name)
{
    IPCService* s = NULL;
    HASH_FIND_STR(_services, name, s);
    return s;
}