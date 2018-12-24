
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

#include "Utils.h"
#include <vka/object_capops.h> // vka_cnode_saveCaller


seL4_Word get_free_slot( KernelTaskContext* context)
{
    seL4_CPtr slot;
    UNUSED int error = vka_cspace_alloc(&context->vka, &slot);
//    assert(!error);
    return error == 0?slot : 0;
}


int cnode_savecaller( KernelTaskContext* context,seL4_CPtr cap)
{
    cspacepath_t path;
    vka_cspace_make_path(&context->vka, cap, &path);
    return vka_cnode_saveCaller(&path);
}


int cnode_delete( KernelTaskContext* context,seL4_CPtr slot)
{
    cspacepath_t path;
    vka_cspace_make_path(&context->vka, slot, &path);
    return vka_cnode_delete(&path);
}
