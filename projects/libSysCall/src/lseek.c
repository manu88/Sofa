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

// off_t lseek(int fd, off_t offset, int whence);
long sofa_lseek(va_list args)
{
        int fd  = va_arg(args, int);
        off_t offset = va_arg(args, off_t);

        int whence  = va_arg(args, int);

        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);

        seL4_SetMR(0, __SOFA_NR_lseek);
        seL4_SetMR(1, fd);
        seL4_SetMR(2, offset);
        seL4_SetMR(3 , whence);


        tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_lseek);
        seL4_Word msg = seL4_GetMR(1);

        return msg;

}
