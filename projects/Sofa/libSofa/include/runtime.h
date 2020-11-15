#pragma once

#include "test.h"
#include "test_init_data.h"



typedef struct
{
    seL4_CPtr ep;
} TLSContext;


void TLSSet( TLSContext* ctx);
TLSContext* TLSGet(void);

void init_allocator(env_t env, test_init_data_t *init_data);
void init_simple(env_t env, test_init_data_t *init_data);

int RuntimeInit(int argc, char *argv[]);


seL4_CPtr getProcessEndpoint(void);
struct env* getProcessEnv(void);

seL4_CPtr getNewThreadEndpoint(void);

void sendThreadExit(seL4_CPtr ep);