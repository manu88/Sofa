#include "LList.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>


int LListList(LList* l)
{
    return list_init(l);
}

int LListSize(const LList* l)
{
    return list_length((list_t*) l);
}

void* LListPut(LList* l)
{
    assert(l);
    if(LListSize(l) == 0)
    {
        return NULL;
    }

    struct list_node *node = l->head;
    assert(node);
    void* dats = node->data;
    if(node->next)
    {
        l->head = node->next;
    }
    else
    {
        l->head = NULL;
    }
    
    free(node);
    return dats;

}