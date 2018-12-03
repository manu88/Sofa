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

//int kill(pid_t pid, int sig);
long sofa_kill(va_list args)
{
    const pid_t pid  = va_arg (args, pid_t);
    const int sig = va_arg (args, int);
    
    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 3);

    seL4_SetMR(0,  __SOFA_NR_kill);
    seL4_SetMR(1 , pid);
    seL4_SetMR(2 , sig);

    tag = seL4_Call(sysCallEndPoint, tag);

    assert(seL4_GetMR(0) == __SOFA_NR_kill);
    msg = seL4_GetMR(1); // ret code
 

    return (int)msg;
}

