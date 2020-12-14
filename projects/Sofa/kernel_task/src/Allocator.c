#include <vka/object.h>
#include "Allocator.h"
#include "utlist.h"


static size_t _kmallocatedMem;

extern uint8_t _env_set;
void *kmalloc(size_t size)
{
    assert(_env_set);
    return malloc(size);
}
void kfree(void *ptr)
{
    free(ptr);   
}
