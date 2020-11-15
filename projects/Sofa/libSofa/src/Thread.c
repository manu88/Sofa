#include <Thread.h>
#include <runtime.h>



static int on_thread(seL4_Word threadPtr, seL4_Word routinePtr, seL4_Word arg, seL4_Word _)
{
    Thread* self = (Thread*)threadPtr;
    assert(self);
    TLSContext _ctx;
    _ctx.ep = self->ep;
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

    t->ep = getNewThreadEndpoint();
    start_helper(getProcessEnv(), &t->th, on_thread, (seL4_Word) t, (seL4_Word) threadMain, (seL4_Word) arg, 0);
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