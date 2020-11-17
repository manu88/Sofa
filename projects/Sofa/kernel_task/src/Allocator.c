#include <vka/object.h>
#include "Allocator.h"
#include "testtypes.h"
#include "utlist.h"

/* list of untypeds to give out to test processes */
static vka_object_t untypeds[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];
/* list of sizes (in bits) corresponding to untyped */
static uint8_t untyped_size_bits_list[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];


static size_t _kmallocatedMem;

/* list of untypeds to give out to test processes */
vka_object_t* getUntypeds()
{
    return untypeds;
}

/* list of sizes (in bits) corresponding to untyped */
uint8_t* GetUntypedSizeBitsList(void)
{
    return untyped_size_bits_list;
}



static int _index = 0;
static FreeRange *untypedsFree = NULL;


static FreeRange* getFirstFreeRange(void)
{
    FreeRange* elt = NULL;
    FreeRange* tmp = NULL;
    LL_FOREACH_SAFE(untypedsFree, elt, tmp)
    {
        LL_DELETE(untypedsFree, elt);
        break;
    }

    return elt;
}

int UntypedsGetFreeRange(UntypedRange* range)
{
    assert(range);


    FreeRange* freeRange = getFirstFreeRange();
    if(freeRange)
    {
        range->start = freeRange->start;
        range->size = freeRange->size;
        kfree(freeRange);
        return 0;
    }

    range->start = _index;
    range->size = UNTYPEDS_PER_PROCESS_BASE;

    _index += range->size;

    return 0;
}


void printUntypedRange(void)
{
    printf("List free range\n");
    FreeRange* elt= NULL;
    LL_FOREACH(untypedsFree, elt)
    {
        printf("\t%i %i\n", elt->start, elt->size);
    }
    printf("End list range\n");

}


void UnypedsGiveBack(const UntypedRange* range)
{
    assert(range);

    FreeRange* newRange = kmalloc(sizeof(FreeRange));
    assert(newRange);
    newRange->start = range->start;
    newRange->size = range->size;
    LL_APPEND(untypedsFree, newRange);
   
}



void *kmalloc(size_t size)
{
    return malloc(size);
}
void kfree(void *ptr)
{
    free(ptr);   
}
