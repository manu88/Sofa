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

// int clock_gettime(clockid_t clk_id, struct timespec *tp);
long sofa_clockgettime(va_list args)
{
        const clockid_t clk_id = va_arg (args, clockid_t);


        if (clk_id <0)
        {
                return -EINVAL;
        }

        struct timespec *tp    = va_arg (args, struct timespec* );

        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
        seL4_SetMR(0, __SOFA_NR_clock_gettime );
        seL4_SetMR(1, clk_id);

        tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_clock_gettime );
        
        seL4_Word retNS = seL4_GetMR(1);

        if (retNS > 0)
        {
                tp->tv_sec  = retNS / 1000000000;
                tp->tv_nsec = retNS - (tp->tv_sec  * 1000000000);

                return 0;
        }

        return retNS;
}
