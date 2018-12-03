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

// int mkdir(const char *pathname, mode_t mode);
long sofa_mkdir(va_list args)
{
        const char* pathname = va_arg(args , char*);
        const mode_t mode    = va_arg(args , mode_t);

        assert(pathname);

        const size_t pathLen = strlen(pathname);

        assert( pathLen );


        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3 + pathLen );

        seL4_SetMR(0, __SOFA_NR_mkdir);
        seL4_SetMR(1,  mode);
        seL4_SetMR(2 , pathLen);

        for( size_t i = 0; i< pathLen; ++i)
        {
                seL4_SetMR(3+i , pathname[i] );
        }

        tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_mkdir );
        return seL4_GetMR(1);


}
