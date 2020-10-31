#pragma once

#include <utils/uthash.h>
#include <sel4/sel4.h>

typedef struct
{
    const char* name;

    seL4_CPtr endpoint;
    seL4_CapRights_t rights;

    UT_hash_handle hh;

} IPCService;


int NameServer_AddService(IPCService* service);
int NameServer_RemoveService(IPCService* service);


IPCService* NameServer_FindService(const char* name);