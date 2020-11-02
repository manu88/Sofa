#include "Thread.h"
#include "proc_ctx.h"
#include <stdio.h>
#include <stdlib.h>

extern ProcessContext* _ctx;


typedef struct {
    void *(*start_routine)(void*);
    void *restrict arg;
}_ThreadArgs;

static int create_thread(pthread_t *restrict thread, void *(*start_routine)(void*), void *restrict arg);

int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void*), void *restrict arg)
{
    assert(_ctx);
    
    return create_thread(thread, start_routine, arg);
}

static void _threadStart(void *arg0, void *arg1, void *ipc_buf)
{
    _ThreadArgs* args = (_ThreadArgs*) arg1;
    assert(args);
// Create & setup TLS context    
    TLSContext threadCtx;
    threadCtx.endpoint = (seL4_CPtr) arg0;
    Thread_init_tls(&threadCtx);

// run user code
    args->start_routine(args->arg);
// Cleanup TLS context
    Thread_init_tls(NULL);
    free(args);
}

static int create_thread(pthread_t *restrict thread, void *(*start_routine)(void*), void *restrict arg)
{
    int error;
    sel4utils_thread_config_t threadConf = thread_config_new(&_ctx->simple);


    threadConf = thread_config_auth(threadConf, _ctx->tcb);


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


    vka_object_t local_endpoint;
    error = vka_alloc_endpoint(&_ctx->vka, &local_endpoint);
    assert(error == 0);
/*
    error = api_tcb_set_space(thread->_thread.tcb.cptr,
                      local_endpoint.cptr,
                      _ctx->root_cnode,
                      api_make_guard_skip_word(seL4_WordBits - _ctx->cspace_size_bits),
                      _ctx->page_directory, seL4_NilData);

    if (error != 0)
    {
        printf("Failed to set fault EP for helper thread. err %i\n", error);
        return error;
    }
*/
    char name[16] = "";
    snprintf(name, 16, "thread-%i", _ctx->pid);
    seL4_DebugNameThread(thread->_thread.tcb.cptr, name);

    _ThreadArgs* args = malloc(sizeof(_ThreadArgs));
    assert(args);
    args->start_routine = start_routine;
    args->arg = arg;
    error = sel4utils_start_thread(&thread->_thread, _threadStart, (void*) local_endpoint.cptr, args, 1);


    if (error != 0)
    {
        printf("error for sel4utils_start_thread %i\n", error);
        return error;
    }

    return 0;

}