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

void processGetPriority(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    int pid = seL4_GetMR(1);
    
    
    Process* proc = ProcessGetByPID(pid);
    int prio= -1;
    int err = -1;
    if( proc)
    {
        err = ProcessGetPriority(proc , &prio);
    }
    
    seL4_SetMR(1 , err);
    seL4_SetMR(2 , prio);
    
    Reply( info);
}

void processSetPriority(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    int pid     = seL4_GetMR(1);
    int newPrio = seL4_GetMR(2);
    printf("[kernel_task] setpriority request for pid %i to %i\n" , pid , newPrio);
    
    int err = -1;
    
    Process* proc = ProcessGetByPID(pid);
    
    // has nice cap and proc is self or a children
    if( ProcessHasCap(sender , SofaCap_Nice)
       && ( proc == sender || ProcessGetParent(proc) == sender ))
    {
        
        
        if( proc)
        {
            err = ProcessSetPriority(proc , newPrio);
        }
        
    }
    seL4_SetMR(1 , err);
    
    Reply(info);
}

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

void processGetIDs(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    SysCallGetIDs_OP op = seL4_GetMR(1);
    
    
    switch (op)
    {
        case SysCallGetIDs_GetPID:
            //printf("GetPID req from %i\n" , sender->pid);
            seL4_SetMR(1, sender->pid );
            Reply( info );
            break;
            
        case SysCallGetIDs_GetPPID:
            seL4_SetMR(1, ProcessGetParent(sender)->pid );
            Reply( info );
            break;
            
        default:
            assert(0);
            break;
    }
}


