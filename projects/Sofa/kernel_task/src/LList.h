#pragma once
#include <utils/list.h>

typedef list_t LList;

int LListInit(LList*l);

int LListSize(const LList* l);

static inline int LListAppend(LList*l ,void* data)
{
    return list_append(l, data);
}

// remove the 1st element and return it
void* LListPut(LList* l);