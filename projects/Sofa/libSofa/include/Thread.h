#pragma once
#include "proc_ctx.h" // temp
#include <sel4utils/thread.h>

#define WORD_STRING_SIZE ((CONFIG_WORD_SIZE / 3) + 1)


/* args provided by the user */
#define HELPER_THREAD_MAX_ARGS 4
/* metadata helpers adds */
#define HELPER_THREAD_META     4
/* total args (user + meta) */
#define HELPER_THREAD_TOTAL_ARGS (HELPER_THREAD_MAX_ARGS + HELPER_THREAD_META)

typedef int (*helper_fn_t)(seL4_Word, seL4_Word, seL4_Word, seL4_Word);

typedef struct helper_thread
{
    sel4utils_thread_t thread;
    vka_object_t local_endpoint;
    seL4_CPtr fault_endpoint;

    void *arg0;
    void *arg1;
    char *args[HELPER_THREAD_TOTAL_ARGS];
    char args_strings[HELPER_THREAD_TOTAL_ARGS][WORD_STRING_SIZE];

    bool is_process;
} helper_thread_t;


void create_helper_thread(ProcessContext *env, helper_thread_t *thread);
/* create a helper with a custom stack size, useful when creating a lot of threads*/
void create_helper_thread_custom_stack(ProcessContext *env, helper_thread_t *thread, size_t stack_pages);

/* Start a helper. Note: arguments to helper processes will be copied into
 * the address space of that process. Do not pass pointers to data only in
 * the local vspace, this will fail. */
void start_helper(ProcessContext * env, helper_thread_t *thread, helper_fn_t entry_point,
                  seL4_Word arg0, seL4_Word arg1, seL4_Word arg2, seL4_Word arg3);

/* wait for a helper thread to finish */
int wait_for_helper(helper_thread_t *thread);
