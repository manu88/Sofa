#include "Thread.h"
#include "proc_ctx.h"
#include <Sofa.h>
#include <stdio.h>
#include <stdlib.h>

extern ProcessContext* _ctx;

NORETURN static void signal_helper_finished(seL4_CPtr local_endpoint, int val)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_SetMR(0, val);

    seL4_Call(local_endpoint, info);
    while (true) {}
}

NORETURN static void helper_thread(int argc, char **argv)
{

    helper_fn_t entry_point = (void *) atol(argv[0]);
    seL4_CPtr local_endpoint = (seL4_CPtr) atol(argv[1]);


    TLSContext __ctx;
    __ctx.endpoint = local_endpoint;// getProcessContext()->mainEndpoint;

    Thread_init_tls(&__ctx);

    seL4_Word args[HELPER_THREAD_MAX_ARGS] = {0};
    for (int i = 2; i < argc && i - 2 < HELPER_THREAD_MAX_ARGS; i++) {
        assert(argv[i] != NULL);
        args[i - 2] = atol(argv[i]);
    }

    /* run the thread */
    int result = entry_point(args[0], args[1], args[2], args[3]);
    signal_helper_finished(local_endpoint, result);
    /* does not return */
}

void create_helper_thread(ProcessContext *env, helper_thread_t *thread)
{
    create_helper_thread_custom_stack(env, thread, BYTES_TO_4K_PAGES(CONFIG_SEL4UTILS_STACK_SIZE));
}

void create_helper_thread_custom_stack(ProcessContext *env, helper_thread_t *thread, size_t stack_pages)
{
    UNUSED int error;

    error = vka_alloc_endpoint(&env->vka, &thread->local_endpoint);
    assert(error == 0);

    thread->is_process = false;
    thread->fault_endpoint = env->mainEndpoint;
    seL4_Word data = api_make_guard_skip_word(seL4_WordBits - env->cspace_size_bits);
    sel4utils_thread_config_t config = thread_config_default(&env->simple, env->root_cnode, data, env->mainEndpoint,
                                                             254 - 1);
    config = thread_config_stack_size(config, stack_pages);
    error = sel4utils_configure_thread_config(&env->vka, &env->vspace, &env->vspace,
                                              config, &thread->thread);
    assert(error == 0);
}

void start_helper(ProcessContext *env, helper_thread_t *thread, helper_fn_t entry_point,
                  seL4_Word arg0, seL4_Word arg1, seL4_Word arg2, seL4_Word arg3)
{
    UNUSED int error;

    seL4_CPtr local_endpoint;


    local_endpoint = thread->local_endpoint.cptr;

    sel4utils_create_word_args(thread->args_strings, thread->args, HELPER_THREAD_TOTAL_ARGS,
                               (seL4_Word) entry_point, local_endpoint,
                               arg0, arg1, arg2, arg3);


    error = sel4utils_start_thread(&thread->thread, (sel4utils_thread_entry_fn)helper_thread,
                                    (void *) HELPER_THREAD_TOTAL_ARGS, (void *) thread->args, 1);
    assert(error == 0);

}


int wait_for_helper(helper_thread_t *thread)
{
    seL4_Word badge;

    api_recv(thread->local_endpoint.cptr, &badge, thread->thread.reply.cptr);
    return seL4_GetMR(0);
}