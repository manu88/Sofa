/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include <assert.h>
#include <vka/capops.h>
#include "Utils.h"

void
sel4osapi_util_copy_cap(
        vka_t *vka, seL4_CPtr src, seL4_CPtr *dest_out)
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





seL4_Word get_free_slot( vka_t *vka)
{
    seL4_CPtr slot;
    UNUSED int error = vka_cspace_alloc(vka, &slot);
    //    assert(!error);
    return error == 0?slot : 0;
}


int cnode_savecaller( vka_t *vka,seL4_CPtr cap)
{
    cspacepath_t path;
    vka_cspace_make_path( vka, cap, &path);
    return vka_cnode_saveCaller(&path);
}


int cnode_delete( vka_t *vka,seL4_CPtr slot)
{
    cspacepath_t path;
    vka_cspace_make_path(vka, slot, &path);
    return vka_cnode_delete(&path);
}



unsigned int
sel4osapi_util_allocate_untypeds(
                                 vka_t *vka, vka_object_t *untypeds, size_t bytes, unsigned int max_untypeds)
{
    unsigned int num_untypeds = 0;
    size_t allocated = 0;
    uint8_t size_bits =  23;
    while (num_untypeds < max_untypeds &&
           allocated + BIT(size_bits) <= bytes &&
           vka_alloc_untyped(vka, size_bits, &untypeds[num_untypeds]) == 0) {
        allocated += BIT(size_bits);
        num_untypeds++;
    }
    return num_untypeds;
}
