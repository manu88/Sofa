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


void processDebug(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const SysCall_Debug_ID debugID = seL4_GetMR(1);
    
    switch (debugID)
    {
        case SysCall_Debug_PS:
            ProcessDump();
            break;
            
        case SysCall_Debug_Sched:
            seL4_DebugDumpScheduler();
            break;

        default:
            assert(0);
            break;
    }
}
