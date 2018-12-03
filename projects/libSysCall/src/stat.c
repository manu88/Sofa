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
#include <sys/stat.h>

#include "SysCallsList.h"

long sofa_stat(va_list args)
{
        const char *pathname = va_arg(args , char*);
        if (pathname == NULL)
        {
                return -EFAULT;
        }
        const size_t pathLen = strlen(pathname);

        if ( pathLen == 0)
        {
                return -ENOENT;
        }

        struct stat *statbuf = va_arg(args , struct stat*);


        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2 + pathLen );

        seL4_SetMR(0, __SOFA_NR_stat);
        seL4_SetMR(1,  pathLen);

        for( size_t i = 0; i< pathLen; ++i)
        {
                seL4_SetMR(2+i , pathname[i] );
        }

        tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_stat );

        const int msgLen = seL4_MessageInfo_get_length(tag)-2;
        long ret = seL4_GetMR(1);

        if (msgLen)
        {
                unsigned long nanos =  seL4_GetMR(2);
 //               printf("stat : got a modTS %lu\n" , seL4_GetMR(2) );
                statbuf->st_mtim.tv_sec  = nanos / 1000000000;
                statbuf->st_mtim.tv_nsec =(nanos % 1000000000); 
        }
        return ret;
}
