#include "Thread.h"
#include "proc_ctx.h"
#include <stdio.h>
#include <stdlib.h>

extern ProcessContext* _ctx;




static int create_thread(pthread_t *restrict thread, void *(*start_routine)(void*), void *restrict arg);

int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void*), void *restrict arg)
{
    assert(_ctx);
    
    return create_thread(thread, start_routine, arg);
}

int pthread_join(pthread_t thread, void **retval)
{
    seL4_Word badge;
    seL4_Wait(thread.local_endpoint.cptr, &badge);
    return seL4_GetMR(0);   
}

static void _threadStart(void *arg0, void *arg1, void *ipc_buf)
{
    pthread_t * th = (pthread_t*) arg1;
    _ThreadArgs* args = &th->arg;
    assert(args);
// Create & setup TLS context    
    TLSContext threadCtx;
    threadCtx.endpoint = (seL4_CPtr) arg0;
    Thread_init_tls(&threadCtx);
    printf("_threadStart\n");
// run user code
    args->start_routine(args->arg);

    printf("[thread] send joint msg\n");

// Cleanup TLS context
    Thread_init_tls(NULL);
    while(1);
// Loop for ever, we should never arrive here!

    seL4_MessageInfo_t info = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_SetMR(0, 43);
    seL4_Call(th->local_endpoint.cptr, info);

    while (1)
    { seL4_Yield(); }
    
}

static int create_thread(pthread_t *restrict thread, void *(*start_routine)(void*), void *restrict arg)
{
    int error;

    sel4utils_thread_config_t threadConf = thread_config_new(&_ctx->simple);

    threadConf = thread_config_auth(threadConf, _ctx->tcb);

    seL4_Word data = seL4_CNode_CapData_new(0, seL4_WordBits - _ctx->cspace_size_bits).words[0];
    threadConf = thread_config_cspace(threadConf, _ctx->root_cnode, data);

    error = sel4utils_configure_thread_config(&_ctx->vka , &_ctx->vspace , &_ctx->vspace , threadConf , &thread->_thread);

    if (error != 0)
    {
        printf("error for sel4utils_configure_thread_config %i\n", error);
        return error;
    }

    error = seL4_TCB_SetPriority(thread->_thread.tcb.cptr, _ctx->tcb,  254);
    if (error != 0)
    {
        printf("error for seL4_TCB_SetPriority %i\n", error);
        return error;
    }

//    error = vka_alloc_endpoint(&_ctx->vka, &thread->local_endpoint);
//    assert(error == 0);

    vka_object_t thread_endpoint;
    error = vka_alloc_endpoint(&_ctx->vka, &thread_endpoint);
    assert(error == 0);

    error = api_tcb_set_space(thread->_thread.tcb.cptr,
                      thread_endpoint.cptr,
                      _ctx->root_cnode,
                      api_make_guard_skip_word(seL4_WordBits - _ctx->cspace_size_bits),
                      _ctx->page_directory, seL4_NilData);

    if (error != 0)
    {
        printf("Failed to api_tcb_set_space for thread. err %i\n", error);
        return error;
    }

    char name[16] = "";
    snprintf(name, 16, "thread-%i", _ctx->pid);
    seL4_DebugNameThread(thread->_thread.tcb.cptr, name);


    thread->arg.start_routine = start_routine;
    thread->arg.arg = arg;
    error = sel4utils_start_thread(&thread->_thread, _threadStart, (void*) thread_endpoint.cptr, thread, 1);


    if (error != 0)
    {
        printf("error for sel4utils_start_thread %i\n", error);
        return error;
    }

    return 0;

}