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

    size_t sizeB = BIT(untyped_size_bits_list[_index]) / 1024;
    printf("[UntypedsGetFreeRange] no slots, take from list at index %i size is %zi\n", _index, sizeB);

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
/*    FreeRange* elt= NULL;
    LL_FOREACH(untypedsFree, elt)
    {
        int eltEnd = elt->start + elt->size;
        if(eltEnd == range->start)
        {
            elt->size += range->size;
            return;
        }
    }
*/
    FreeRange* newRange = kmalloc(sizeof(FreeRange));
    assert(newRange);
    newRange->start = range->start;
    newRange->size = range->size;
    LL_APPEND(untypedsFree, newRange);
    printUntypedRange();
   
}



void *kmalloc(size_t size)
{
    return malloc(size);
}
void kfree(void *ptr)
{
    free(ptr);   
}


static size_t _numPagesAlloc = 0;

size_t getNumPagesAlloc()
{
    return _numPagesAlloc;
}

vspace_new_pages_fn old_vspace_new_pages_fn;
vspace_unmap_pages_fn old_vspace_unmap_pages_fn;
vspace_new_pages_at_vaddr_fn old_vspace_new_pages_at_vaddr_fn;

static void* _vspace_new_pages_fn(vspace_t *vspace, seL4_CapRights_t rights,
                                     size_t num_pages, size_t size_bits)
{
    void* p = old_vspace_new_pages_fn(vspace, rights, num_pages, size_bits);
    if(p)
    {
        _numPagesAlloc += num_pages;
    }
    else
    {
        printf("CURRENT ALLOC'D PAGES COUNT %zi\n", _numPagesAlloc);
    }
    
    return p;
}

static void _vspace_unmap_pages_fn(vspace_t *vspace, void *vaddr, size_t num_pages,
                                      size_t size_bits, vka_t *free)
{
    if(free == VSPACE_FREE)
    {
        _numPagesAlloc -= num_pages;
    }
    return old_vspace_unmap_pages_fn(vspace, vaddr, num_pages, size_bits, free);
}


static int _vspace_new_pages_at_vaddr_fn(vspace_t *vspace, void *vaddr, size_t num_pages,
                                            size_t size_bits, reservation_t reservation, bool can_use_dev)
{
    
    int r = old_vspace_new_pages_at_vaddr_fn(vspace, vaddr, num_pages, size_bits, reservation, can_use_dev);
    if(r == 0)
    {
        _numPagesAlloc += num_pages;
    }
    return r;
}

void installEnvCallbacks()
{
    KernelTaskContext* ctx = getKernelTaskContext();

    old_vspace_new_pages_fn = ctx->vspace.new_pages;
    ctx->vspace.new_pages = _vspace_new_pages_fn;

    old_vspace_unmap_pages_fn = ctx->vspace.unmap_pages;
    ctx->vspace.unmap_pages = _vspace_unmap_pages_fn;

    old_vspace_new_pages_at_vaddr_fn = ctx->vspace.new_pages_at_vaddr;
    ctx->vspace.new_pages_at_vaddr = _vspace_new_pages_at_vaddr_fn;
}