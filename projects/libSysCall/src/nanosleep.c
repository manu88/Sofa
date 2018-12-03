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

#include <utils/time.h>
#include "SysCallsList.h"

long sofa_nanosleep(va_list args)
{
    struct timespec *req = va_arg(args, struct timespec*);
    /* construct a sleep call */
    int millis = req->tv_sec * MS_IN_S;
    millis += req->tv_nsec / NS_IN_MS;
//    printf("Usleep %i ms\n",millis);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, __SOFA_NR_nanosleep);
    seL4_SetMR(1 , millis);

    tag = seL4_Call(sysCallEndPoint, tag);

    assert(seL4_MessageInfo_get_length(tag) >=1 );

    assert(seL4_GetMR(0) == __SOFA_NR_nanosleep);

    return seL4_GetMR(1);
}
