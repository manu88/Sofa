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
#pragma once
#include <sel4/sel4.h>
#include "sel4process.h"

seL4_CPtr process_copy_cap_into(sel4utils_process_t *process, seL4_Word cap_badge, vka_t *parent_vka, seL4_CPtr cap, seL4_CapRights_t rights);

void sel4osapi_util_copy_cap(vka_t *vka, seL4_CPtr src, seL4_CPtr *dest_out);

void util_copy_cap(vka_t *vka, seL4_CPtr src, seL4_CPtr *dest_out);

seL4_Word get_free_slot( vka_t *vka);
void set_cap_receive_path(vka_t *vka, seL4_CPtr slot);


int cnode_move(vka_t *vka, seL4_CPtr src, seL4_CPtr dest);
int cnode_savecaller( vka_t *vka, seL4_CPtr cap);
int cnode_delete( vka_t *vka, seL4_CPtr slot);
int cnode_mint(vka_t *vka, seL4_CPtr src, seL4_CPtr dest, seL4_CapRights_t rights, seL4_Word badge);
int is_slot_empty(vka_t *vka, seL4_Word slot);