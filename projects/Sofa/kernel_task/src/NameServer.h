#pragma once
#include "Process.h"
#include <utils/uthash.h>

typedef struct 
{
    char* name;
    Process* owner;

    UT_hash_handle hh;
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