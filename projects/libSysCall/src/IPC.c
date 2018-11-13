
#include "SysClient.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>

#ifdef __APPLE__

static void seL4_SetMR( uint32_t index ,seL4_Word word )
{
    
}
#endif

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))
#define ROUND_DOWN(N, S) (((N) / (S)) * (S))

static uint32_t _rpc_mr;


int IPCMessageReset(void)
{
    _rpc_mr = 0;
    return 1;
}


static uint32_t rpc_marshall(uint32_t cur_mr, const char *str, uint32_t slen)
{
    assert(str);
    if (slen == 0)
    {
        return cur_mr;
    }
    
    int i;
    for (i = 0; i < ROUND_DOWN(slen, 4); i+=4, str+=4) {
        seL4_SetMR(cur_mr++, *(seL4_Word*) str);
    }
    if (i != slen) {
        seL4_Word w = 0;
        memcpy(&w, str, slen - i);
        seL4_SetMR(cur_mr++, w);
    }
    
    return cur_mr;
}

void IPCPushWord(seL4_Word word)
{
    seL4_SetMR(_rpc_mr++, word);
}

void IPCPushString(const char* str)
{
    uint32_t slen = (uint32_t) strlen(str);
    IPCPushWord(slen);
    _rpc_mr = rpc_marshall(_rpc_mr, str, slen);
}
