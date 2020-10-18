#include "Sofa.h"
#include "sys_calls.h"
#include "proc_ctx.h"
#include <sel4/sel4.h>
#include <stddef.h>
#include <stdio.h>


static seL4_CPtr _endpoint = 0;
static ProcessContext* _ctx = NULL;

static size_t write_buf(void *data, size_t count)
{
    if(_endpoint == 0)
    {
        return 0;
    }
    for (int i=0;i<count ; i++)
    {
        struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
        seL4_SetMR(0, SofaSysCall_PutChar);
        seL4_SetMR(1, ((char*)data)[i]);
        seL4_Send(_endpoint, msg);
    }
    return count;
}

static ProcessContext* sendInit()
{
    struct seL4_MessageInfo msg =  seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,2);
    seL4_SetMR(0, SofaSysCall_InitProc);
    seL4_Call(_endpoint, msg);

    ProcessContext* ctx = seL4_GetMR(1);
    return ctx;
}

int ProcessInit(void* endpoint)
{
    _endpoint = endpoint;

    sel4muslcsys_register_stdio_write_fn(write_buf);

    printf("Send init RPC call\n");
    _ctx = sendInit();
    printf("Test %i\n", _ctx->test);
    return 0;
}