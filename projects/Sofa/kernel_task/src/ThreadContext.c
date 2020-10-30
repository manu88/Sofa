#include "ThreadContext.h"

static ThreadContext* _waitingList = NULL;


void WaitList_Add(ThreadContext* context)
{
    HASH_ADD_PTR(_waitingList, endpoint, context);
}