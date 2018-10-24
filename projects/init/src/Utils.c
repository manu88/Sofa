
#include "Utils.h"
#include <vka/object_capops.h> // vka_cnode_saveCaller


seL4_Word get_free_slot( InitContext* context)
{
    seL4_CPtr slot;
    UNUSED int error = vka_cspace_alloc(&context->vka, &slot);
    assert(!error);
    return slot;
}


int cnode_savecaller( InitContext* context,seL4_CPtr cap)
{
    cspacepath_t path;
    vka_cspace_make_path(&context->vka, cap, &path);
    return vka_cnode_saveCaller(&path);
}


int cnode_delete( InitContext* context,seL4_CPtr slot)
{
    cspacepath_t path;
    vka_cspace_make_path(&context->vka, slot, &path);
    return vka_cnode_delete(&path);
}
