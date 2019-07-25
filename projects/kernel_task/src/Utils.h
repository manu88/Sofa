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
