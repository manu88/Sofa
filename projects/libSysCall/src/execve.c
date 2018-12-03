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


#include <string.h>
#include "SysCallsList.h"


//int execve(const char *filename, char *const argv[], char *const envp[]);
long sofa_execve(va_list args)
{
    const char* filename = va_arg(args , char*);

    if (!filename)
    {
        return -EFAULT;
    }

//    printf("Execve : '%s'\n", filename);
    seL4_MessageInfo_t tag;
    seL4_Word msg;
    
    tag = seL4_MessageInfo_new(0, 0, 0, 1/*syscall num*/ + strlen(filename) );
    seL4_SetMR(0, __SOFA_NR_execve);

    for( int i=0;i<strlen(filename);++i)
    {
        seL4_SetMR(1 +i , filename[i]);
    }
    tag = seL4_Call(sysCallEndPoint, tag);

    assert(seL4_GetMR(0) == __SOFA_NR_execve);
    msg = seL4_GetMR(1); // ret code
    
    return (int)msg;

//    return ENOSYS;
}

