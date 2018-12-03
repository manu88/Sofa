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

static long doRead(int fd, void *buf, size_t count , int expectedNodeType);


static long doRead(int fd, void *buf, size_t count , int expectedNodeType)
{
        if (fd <0)
        {
                return -EBADF;
        }
//      printf("Read request fd %i count %lu\n", fd , count);

        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);
        seL4_SetMR(0, __SOFA_NR_read);
        seL4_SetMR(1, fd);
        seL4_SetMR(2, count);
        seL4_SetMR(3, expectedNodeType);

        tag = seL4_Call(sysCallEndPoint, tag);

        assert(seL4_MessageInfo_get_length(tag) >= 2);

        assert(seL4_GetMR(0) == __SOFA_NR_read);


        ssize_t ret = seL4_GetMR(1);// seL4_MessageInfo_get_length(tag) - 2;

        if (ret > 0)
        {
                char* b = (char*) buf;
                for(int i= 0; i<ret;++i)
                {
                        b[i] = seL4_GetMR(2+i);
                }
                b[ret] = 0;
        }

        return ret;
}

// ssize_t read(int fd, void *buf, size_t count);
long sofa_read(va_list args)
{
        const int fd    = va_arg(args , int);
        void*     buf   = va_arg(args , void*);
        size_t    count = va_arg(args , size_t);
        return doRead(fd , buf, count , 1);
}


//int getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count);
long sofa_getdents64(va_list args)
{
        const int fd                = va_arg (args, int);

        struct dirent64 *dirp = va_arg (args, struct dirent64 *);
        unsigned int count          = va_arg (args,unsigned int);

        memset(dirp , 0 , count);
        return doRead(fd , dirp, count , 2);
}
