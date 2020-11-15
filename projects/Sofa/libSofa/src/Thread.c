#include <Thread.h>
#include <runtime.h>



static int on_thread(seL4_Word threadPtr, seL4_Word routinePtr, seL4_Word arg, seL4_Word ipcBufAddr)
{
    Thread* self = (Thread*)threadPtr;
    assert(self);
    TLSContext _ctx;
    memset(&_ctx, 0, sizeof(TLSContext));
    _ctx.ep = self->ep;
    _ctx.buffer = (uint8_t*) ipcBufAddr;
    TLSSet(&_ctx);

    void* ret = ((start_routine) routinePtr)((void*)arg);

    sendThreadExit(self->ep);
    TLSSet(NULL);
    self->ret = ret;
    return 0;
}

int ThreadInit(Thread* t, start_routine threadMain, void* arg)
{
    create_helper_thread(getProcessEnv(), &t->th);
    uint8_t* ipcBufAddr = NULL;
    t->ep = getNewThreadEndpoint(&ipcBufAddr);
    start_helper(getProcessEnv(), &t->th, on_thread, (seL4_Word) t, (seL4_Word) threadMain, (seL4_Word) arg, ipcBufAddr);
    return 0;
}


int ThreadJoin(Thread* t, void **retval)
{
    wait_for_helper(&t->th);
    if(retval)
    {
        *retval = t->ret;
    }
    return 0;
}