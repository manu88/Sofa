#include "Utils.h"
#include <vka/ipcbuffer.h>
#include <vka/capops.h>

int is_slot_empty(ProcessContext* context, seL4_Word slot)
{
    int error;

    error = cnode_move(context, slot, slot);

    /* cnode_move first check if the destination is empty and raise
     * seL4_DeleteFirst is it is not
     * The is check if the source is empty and raise seL4_FailedLookup if it is
     */
    if (error != seL4_DeleteFirst && error != seL4_FailedLookup)
    {
        printf("is_slot_empty error=%i\n", error);
    }
    assert(error == seL4_DeleteFirst || error == seL4_FailedLookup);
    return (error == seL4_FailedLookup);
}

seL4_Word get_free_slot(ProcessContext* context)
{
    seL4_CPtr slot;
    UNUSED int error = vka_cspace_alloc(&context->vka, &slot);
    assert(!error);
    return slot;
}

int cnode_mint(ProcessContext* context, seL4_CPtr src, seL4_CPtr dest, seL4_CapRights_t rights, seL4_Word badge)
{
    cspacepath_t src_path, dest_path;

    vka_cspace_make_path(&context->vka, src, &src_path);
    vka_cspace_make_path(&context->vka, dest, &dest_path);
    return vka_cnode_mint(&dest_path, &src_path, rights, badge);
}


int cnode_move(ProcessContext* context, seL4_CPtr src, seL4_CPtr dest)
{
    cspacepath_t src_path, dest_path;

    vka_cspace_make_path(&context->vka, src, &src_path);
    vka_cspace_make_path(&context->vka, dest, &dest_path);
    return vka_cnode_move(&dest_path, &src_path);
}