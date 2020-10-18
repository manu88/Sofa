#include "utils.h"
#include <vka/capops.h>


seL4_CPtr process_copy_cap_into(sel4utils_process_t *process, seL4_Word cap_badge, vka_t *parent_vka, seL4_CPtr cap, seL4_CapRights_t rights)
{
    seL4_CPtr minted_cap;
    cspacepath_t src_path;

    vka_cspace_make_path(parent_vka, cap, &src_path);
    minted_cap = sel4utils_mint_cap_to_process(process, src_path, rights, cap_badge);
    assert(minted_cap != 0);

    return minted_cap;
}


void util_copy_cap(vka_t *vka, seL4_CPtr src, seL4_CPtr *dest_out)
{
    int error = 0;
    cspacepath_t copy_src, copy_dest;
    /* copy the cap to map into the remote process */
    vka_cspace_make_path(vka, src, &copy_src);
    error = vka_cspace_alloc(vka, dest_out);
    assert(error == 0);
    vka_cspace_make_path(vka, *dest_out, &copy_dest);
    error = vka_cnode_copy(&copy_dest, &copy_src, seL4_AllRights);
    assert(error == 0);
}