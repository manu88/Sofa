#include "SyscallTable.h"
#include "../utils.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>


typedef struct
{
    driver_env_t *env;
    unsigned int timerID;
    Process* process;
    seL4_Word cap;

} ReplyObject;


static int sleep_callback(uintptr_t token)
{
    ReplyObject* reply = (ReplyObject*) token;
    assert(reply->process);
    printf("Timer callback from process %i\n", ProcessGetPID(reply->process));

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Sleep);
    seL4_SetMR(1, 0); // sucess

    seL4_Send(reply->cap, tag);

    cnode_delete(&reply->env->vka, reply->cap);
    tm_free_id(&reply->env->tm, reply->timerID);
    free(reply);
    return 0;
}


void Syscall_sleep(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info)
{
    Process* callingProcess = caller->process;
    assert(callingProcess);
    printf("Sleep request from %s %i (thread %p)\n",
           ProcessGetName(callingProcess),
           ProcessGetPID(callingProcess),
           caller == &callingProcess->main? 0 : (void*) caller
           );

    unsigned int timerID = 0;
    int error = tm_alloc_id(&env->tm, &timerID);
    if (error)
    {
        printf("Unable to alloc timer id err=%i\n", error);

        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }
    printf("Allocated timer ID=%i\n", timerID);


    seL4_Word slot = get_free_slot(&env->vka);
    error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        printf("Unable to save caller err=%i\n", error);
        tm_free_id(&env->tm, timerID);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }


    ReplyObject* reply = malloc(sizeof(ReplyObject));
    if(reply == NULL)
    {
        printf("Unable to allocate a reply object\n");
        tm_free_id(&env->tm, timerID);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -ENOMEM);
        seL4_Reply(info);
        return;

    }
    assert(reply);
    memset(reply, 0, sizeof(ReplyObject));
    error = tm_register_cb(&env->tm, TIMEOUT_RELATIVE, seL4_GetMR(1) * NS_IN_MS, 0, timerID , sleep_callback, (uintptr_t) reply);
    if(error)
    {
        printf("tm_register_failed, err=%i\n", error);
        tm_free_id(&env->tm, timerID);
        cnode_delete(&env->vka, slot);
        free(reply);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    reply->env = env;
    reply->timerID = timerID;
    reply->process = callingProcess;
    reply->cap = slot;

}