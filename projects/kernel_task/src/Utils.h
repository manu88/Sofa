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

#pragma once

#include <vka/object.h>

/*
 * Create a copy of a cap to a free slot within the same CSpace.
 */
void sel4osapi_util_copy_cap(vka_t *vka, seL4_CPtr src, seL4_CPtr *dest_out);

seL4_Word get_free_slot( vka_t *vka);

int cnode_savecaller( vka_t *vka,seL4_CPtr cap);

int cnode_delete( vka_t *vka,seL4_CPtr slot);

/*
 * Allocate untypeds on the supplied VKA, till either a certain
 * number of bytes is allocated or a certain number of untyped objects
 * is allocated.,
 *
 * @param [in]  vka          the VKA on which to allocate the untypeds.
 * @param [out] untypeds     array of untyped objects allocated by the operation.
 * @param [in]  bytes        the maximum number of bytes of untyped memory to allocate
 * @param [in]  max_untypeds the maximum number of untypeds objects to allocate
 *
 * @return the number of untyped objects that were allocated.
 *
 */
unsigned int sel4osapi_util_allocate_untypeds(vka_t *vka, vka_object_t *untypeds, size_t bytes, unsigned int max_untypeds);
