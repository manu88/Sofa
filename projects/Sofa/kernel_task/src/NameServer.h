#pragma once
//#include "Process.h"
#include <utils/uthash.h>
#include <sel4/sel4.h>


typedef struct _Process Process;

typedef struct _Service
{
    char* name;
    Process* owner;

    UT_hash_handle hh;

    seL4_CPtr endpoint;
    seL4_CPtr baseEndpoint;
} Service;


Service* NameServerGetServices(void);


#define FOR_EACH_SERVICE(service, tmp) HASH_ITER(hh, NameServerGetServices(), service, tmp)


static inline void ServiceInit(Service* s, Process* owner)
{
    memset(s, 0, sizeof(Service));
    s->owner = owner;
}


int NameServerInit(void);


int NameServerRegister(Service* service);
int NameServerUnregister(Service* s);


Service* NameServerFind(const char *name);