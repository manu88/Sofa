/*
 * This file is part of the Sofa project
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
#include <Sofa.h>
#include "SyscallTable.h"
#include "Reboot.h"


void SysCall_Reboot(Thread* caller, seL4_MessageInfo_t info)
{
    RebootMode mode = seL4_GetMR(1);
    KLOG_INFO("Syscall reboot, code %i\n", mode);

    int ret = -1;
    if(mode == RebootMode_Shutdown)
    {
        ret = 0;
    }

    seL4_SetMR(1, ret);
    seL4_Reply(info);

    if(mode == RebootMode_Shutdown)
    {
        doShutdown();
    }
} 