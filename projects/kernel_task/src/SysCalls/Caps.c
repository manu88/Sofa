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

#include "../ProcessSysCall.h"


void processCapOp(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    CapOperation capOP = seL4_GetMR(1);
    
    SofaCapabilities caps = seL4_GetMR(2);
    switch (capOP)
    {
        case CapOperation_Drop:
            printf("[kernel_task] Drop cap request %i\n" , caps);
            sender->caps.caps &= ~caps;
            ProcessDumpCaps(sender);
            seL4_SetMR(1, -1 );
            Reply( info );
            break;
            
        case CapOperation_Acquire:
            printf("[kernel_task] Acquire cap request %i\n" , caps);
            
            sender->caps.caps |= caps;
            ProcessDumpCaps(sender);
            seL4_SetMR(1, -1 );
            Reply( info );
            break;
        default:
            assert(0);
            break;
    }
}
