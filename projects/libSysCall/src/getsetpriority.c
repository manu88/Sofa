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

//int getpriority(int which, id_t who);
long sofa_getpriority(va_list args)
{
    // The range of the nice value is +19 (low priority) to -20 (high priority)
    int which  = va_arg(args, int);
    id_t who   = va_arg(args, id_t);


    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 3);

    seL4_SetMR(0, __SOFA_NR_getpriority);
    seL4_SetMR(1 , which);
    seL4_SetMR(2 , who);

    tag = seL4_Call(sysCallEndPoint, tag);
    assert(seL4_GetMR(0) == __SOFA_NR_getpriority);
    msg = seL4_GetMR(1);

    return msg;

}

//int setpriority(int which, id_t who, int prio);
long sofa_setpriority(va_list args)
{
    // The range of the nice value is +19 (low priority) to -20 (high priority)
    int which  = va_arg(args, int);
    id_t who   = va_arg(args, id_t);
    int prio   = va_arg(args, int);
    int mappedPrio = (-prio + 19)*6;


    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 4);

    seL4_SetMR(0, __SOFA_NR_setpriority);
    seL4_SetMR(1 , which);
    seL4_SetMR(2 , who);
    seL4_SetMR(3 , mappedPrio);

    tag = seL4_Call(sysCallEndPoint, tag);
    assert(seL4_GetMR(0) == __SOFA_NR_setpriority);
    msg = seL4_GetMR(1);

    return msg;

}
