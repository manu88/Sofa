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

static void Abort( int status)
{
//    printf("Abort called with status %i from %li\n" , status , getpid() );

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, __SOFA_NR_exit);
    seL4_SetMR(1 , status);
    seL4_Send(sysCallEndPoint , tag);
    while(1){} //  we never return from Abort
}

long sofa_exit(va_list args)
{
    int status =  va_arg(args,int);
    Abort( status);
    return 0;
}
