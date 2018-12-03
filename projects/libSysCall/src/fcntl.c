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

#include <fcntl.h>
#include "SysCallsList.h"

// int fcntl(int fd, int cmd, ... /* arg */ );
long sofa_fcntl(va_list args)
{
        const int fd  = va_arg (args, int);
        const int cmd = va_arg (args, int);
        
        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);

        seL4_SetMR(0, __SOFA_NR_fcntl);
        seL4_SetMR(1, fd);
        seL4_SetMR(2, cmd);

        switch(cmd)
        {
                case F_SETFD:
                {
                        int flag = va_arg (args, int);
                        seL4_SetMR(3, flag);
//                      printf("sys_fcntl flag %i\n" , flag);
                }
                break;

                // passthrougth cases
                case F_GETFD:

                break;
                
                default:
                return -ENOSYS;
        }


        tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_fcntl );
        return seL4_GetMR(1);

}

