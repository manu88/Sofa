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

#include <sel4/types.h>
#include <vspace/vspace.h>
#include "utlist.h"


#define VM_PAGE_SIZE 4096

typedef seL4_Word vaddr_t;

typedef struct _VMRegion
{
    vaddr_t start;
    size_t size;
    seL4_CapRights_t rights;

    reservation_t _reservation;
    uint8_t hasReservation:1;

    vka_object_t _frame;

    struct _VMRegion *prev, *next;
} VMRegion;


int VMRegionInit(VMRegion* r);


typedef struct vspace vspace_t;
typedef struct
{
    vspace_t *vspace;
    VMRegion* regions;
} VMSpace;

int VMSpaceInit(VMSpace *s, vspace_t *vspace);

void VMSpacePrint(VMSpace* s);

void* VMSpaceReserveRange(VMSpace *s, size_t bytes, seL4_CapRights_t rights);

int VMSpaceSplitRegion(VMSpace* s, VMRegion* reg, vaddr_t allocatedStart, size_t allocatedSize);

int VMSpaceUnMap(VMSpace* s, vaddr_t vaddr, size_t numPages);
VMRegion* VMSpaceGetRegion(VMSpace* s, vaddr_t addr);