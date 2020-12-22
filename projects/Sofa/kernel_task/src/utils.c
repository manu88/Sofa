/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils.h"
#include <vka/capops.h>
#include <vka/ipcbuffer.h>

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


void set_cap_receive_path(vka_t *vka, seL4_CPtr slot)
{
    cspacepath_t path;
    vka_cspace_make_path(vka, slot, &path);
    vka_set_cap_receive_path(&path);
}

seL4_Word get_free_slot(vka_t *vka)
{
    seL4_CPtr slot;
    int error = vka_cspace_alloc(vka, &slot);
    //    assert(!error);
    return error == 0?slot : 0;
}

int cnode_mint(vka_t *vka, seL4_CPtr src, seL4_CPtr dest, seL4_CapRights_t rights, seL4_Word badge)
{
    cspacepath_t src_path, dest_path;

    vka_cspace_make_path(vka, src, &src_path);
    vka_cspace_make_path(vka, dest, &dest_path);
    return vka_cnode_mint(&dest_path, &src_path, rights, badge);
}

int cnode_savecaller(vka_t *vka, seL4_CPtr cap)
{
    cspacepath_t path;
    vka_cspace_make_path( vka, cap, &path);
    return vka_cnode_saveCaller(&path);
}

int cnode_delete(vka_t *vka, seL4_CPtr slot)
{
    cspacepath_t path;
    vka_cspace_make_path(vka, slot, &path);
    return vka_cnode_delete(&path);
}

int cnode_move(vka_t *vka, seL4_CPtr src, seL4_CPtr dest)
{
    cspacepath_t src_path, dest_path;

    vka_cspace_make_path(vka, src, &src_path);
    vka_cspace_make_path(vka, dest, &dest_path);
    return vka_cnode_move(&dest_path, &src_path);
}

int is_slot_empty(vka_t *vka, seL4_Word slot)
{
    int error;

    error = cnode_move(vka, slot, slot);

    /* cnode_move first check if the destination is empty and raise
     * seL4_DeleteFirst is it is not
     * The is check if the source is empty and raise seL4_FailedLookup if it is
     */
    assert(error == seL4_DeleteFirst || error == seL4_FailedLookup);
    return (error == seL4_FailedLookup);
}