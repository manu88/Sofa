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
#include <string.h>
#include <math.h>
#include <vspace/vspace.h>
#include <sel4utils/vspace_internal.h>
#include "VMSpace.h"
#include "Log.h"
#include "Environ.h"

int VMRegionInit(VMRegion* r)
{
    memset(r, 0, sizeof(VMRegion));
    return 0;
}

int VMSpaceInit(VMSpace *s, vspace_t *vspace)
{
    memset(s, 0, sizeof(VMSpace));
    s->vspace = vspace;
    return 0;
}

void VMSpacePrint(VMSpace* s)
{
    KLOG_DEBUG("-> Start region list\n");
    VMRegion* reg = NULL;
    DL_FOREACH(s->regions, reg)
    {
        KLOG_DEBUG("Region %X %X reserved? %i frame=%X\n", reg->start, reg->size, reg->hasReservation, reg->_frame.cptr);
    }
    KLOG_DEBUG("<- End region list\n");
}

int VMSpaceUnMap(VMSpace* s, vaddr_t vaddr, size_t numPages)
{
    VMRegion* reg = VMSpaceGetRegion(s, vaddr);
    if(reg == NULL)
    {
        return -1;
    }
    if(reg->hasReservation)
    {
        vspace_free_reservation(s->vspace, reg->_reservation);
        reg->hasReservation = 0;
    }
    if(reg->_frame.cptr)
    {
        KLOG_DEBUG("Unmap %zi page(s) at  %X\n", numPages, vaddr);
        sel4utils_unmap_pages(s->vspace, vaddr, numPages, PAGE_BITS_4K, VSPACE_FREE);
#if 0 // this crashes
        MainVKALock();
        KLOG_DEBUG("free frame object\n");
        vka_free_object(getMainVKA(), &reg->_frame);
        MainVKAUnlock();
#endif
        KLOG_DEBUG("Good to go\n");
    }
    
    DL_DELETE(s->regions, reg);
    free(reg);
    return 0;
}

void* VMSpaceReserveRange(VMSpace* s, size_t bytes, seL4_CapRights_t rights)
{
    void* vaddr = NULL;
    const size_t numPages = (size_t) ceil(bytes / VM_PAGE_SIZE);

    //reservation_t reservation = vspace_reserve_range_aligned(&p->native.vspace, bytes, BYTES_TO_SIZE_BITS(bytes), rights, 0/*cacheable*/, &vaddr);
    reservation_t reservation = vspace_reserve_range(s->vspace, bytes, rights, 0/*cacheable*/, &vaddr);
    if(reservation.res == NULL)
    {
        return NULL;
    }
    VMRegion *newRegion = malloc(sizeof(VMRegion));
    VMRegionInit(newRegion);
    assert(newRegion);

    newRegion->size = bytes;
    newRegion->start = (vaddr_t) vaddr;
    newRegion->_reservation = reservation;
    newRegion->hasReservation = 1;
    newRegion->rights = rights;

    DL_APPEND(s->regions, newRegion);
    sel4utils_res_t * res = reservation_to_res(reservation);
    KLOG_DEBUG("process_reserve_range start %X end %X, vaddr=%p\n", res->start, res->end, vaddr);
    VMSpacePrint(s);
    return vaddr;
}

VMRegion* VMSpaceGetRegion(VMSpace* s, vaddr_t addr)
{
    VMRegion* reg = NULL;
    DL_FOREACH(s->regions, reg)
    {
        if(addr >= reg->start && addr< reg->start + reg->size)
        {
            return reg;
        }
    }
    return NULL;
}


int VMSpaceSplitRegion(VMSpace* s, VMRegion* reg, vaddr_t allocatedStart, size_t allocatedSize)
{
    KLOG_DEBUG("Split region %X size %X at %X size %X\n", reg->start, reg->size, allocatedStart, allocatedSize);
    if(reg->start == allocatedStart)
    {
        const size_t remainingSize = reg->size - allocatedSize;
        const vaddr_t newRegionStart = reg->start + allocatedSize;
        KLOG_DEBUG("Slit mode 1: allocated is at start, will create a new region at %X size %X\n", newRegionStart, remainingSize);
        reg->size = allocatedSize;


        VMRegion* newReg = malloc(sizeof(VMRegion));
        assert(newReg);
        VMRegionInit(newReg);
        newReg->_reservation = reg->_reservation;
        newReg->hasReservation = 1;
        sel4utils_move_resize_reservation(s->vspace, newReg->_reservation, newRegionStart, remainingSize);
        newReg->start = newRegionStart;
        newReg->size = remainingSize;
        DL_APPEND(s->regions, newReg);
    }
    return 0;
}