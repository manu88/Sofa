#include "Process.h"
#include "utils.h"


void ThreadCleanupTimer(Thread* t, driver_env_t *env)
{
    tm_deregister_cb(&env->tm, t->timerID);
    tm_free_id(&env->tm, t->timerID);
    cnode_delete(&env->vka, t->replyCap);
    t->replyCap = 0;
}