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
#include "VMSpace.h"
#include <vspace/vspace.h>
#include <sel4utils/vspace_internal.h>
#include "Log.h"


int VMSpaceInit(VMSpace *s, vspace_t *vspace)
{
    memset(s, 0, sizeof(VMSpace));
    s->vspace = vspace;
    return 0;
}

void* VMSpaceReserveRange(VMSpace* s, size_t bytes, seL4_CapRights_t rights)
{
    void* vaddr = NULL;
    const size_t numPages = (size_t) ceil(bytes / 4096);

    //reservation_t reservation = vspace_reserve_range_aligned(&p->native.vspace, bytes, BYTES_TO_SIZE_BITS(bytes), rights, 0/*cacheable*/, &vaddr);
    reservation_t reservation = vspace_reserve_range(s->vspace, bytes, rights, 0/*cacheable*/, &vaddr);
    assert(reservation.res != NULL);
    sel4utils_res_t * res = reservation_to_res(reservation);
    KLOG_DEBUG("process_reserve_range start %X end %X, vaddr=%p\n", res->start, res->end, vaddr);
    return vaddr;
}