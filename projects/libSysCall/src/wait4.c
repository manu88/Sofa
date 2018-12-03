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

#include "SysCallsList.h"



//pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);
long sofa_wait4(va_list args)
{
    const pid_t pid = va_arg(args , pid_t);
    int *wstatus    = va_arg(args , int*);
    const int options     = va_arg(args , int);
    struct rusage *rusage = va_arg(args ,struct rusage *);

    seL4_MessageInfo_t tag;
    seL4_Word msg;
    
    tag = seL4_MessageInfo_new(0, 0, 0, 3 );
    seL4_SetMR(0, __SOFA_NR_wait4);
    seL4_SetMR(1, pid);
    seL4_SetMR(2, options);

    tag = seL4_Call(sysCallEndPoint, tag);

    assert(seL4_GetMR(0) == __SOFA_NR_wait4);
    msg = seL4_GetMR(1); // ret code}

    return msg;
}

