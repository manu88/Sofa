#include "ProcessList.h"
#include "utils.h"


void ThreadCleanupTimer(Thread* t)
{
    KernelTaskContext* env = getKernelTaskContext();
    tm_deregister_cb(&env->tm, t->_base.timerID);
    tm_free_id(&env->tm, t->_base.timerID);
    cnode_delete(&env->vka, t->_base.replyCap);
    t->_base.replyCap = 0;
}