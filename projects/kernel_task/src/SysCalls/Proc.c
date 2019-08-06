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
#include "../Bootstrap.h"
#include "../Utils.h"

void processExit(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    int retCode = seL4_GetMR(1);
    
    sender->retCode  = retCode;
    
    printf("[kernel_task] program exited with code %i \n" , retCode);
    
    ProcessKill(sender, SofaSignal_None);
    
    // nothing to return
}

void processKill(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    int pidToKill = seL4_GetMR(1);
    printf("kill pid %i\n" , pidToKill);
    int err = -1;
    
    if( pidToKill > 1) // can't kill init or kernel_task
    {
        Process* procToKill = ProcessGetByPID(pidToKill);
        
        if (procToKill)
        {
            // do we have SofaCap_Kill to kill ALL processes, or is the process to kill a child of the sender
            // XXX Allow to kill self?
            if (  (ProcessGetParent(procToKill) == sender ) || (ProcessHasCap(sender , SofaCap_Kill) ))
            {
                err = ProcessKill(procToKill ,SofaSignal_Kill);
            }
        }
        
    }
    seL4_SetMR(0 , SysCall_Kill);
    seL4_SetMR(1 ,err);
    Reply( info);
}



void processSpawn(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    printf("[kernel_task] spawn '%s'\n", ProcessGetIPCBuffer(sender));
    
    int err = -1;
    
    if (ProcessHasCap(sender , SofaCap_Spawn) )
    {
        Process* newProc = malloc(sizeof(Process));
        if( newProc)
        {
            ProcessInit(newProc);
            err = ProcessStart(newProc, ProcessGetIPCBuffer(sender) ,&getKernelTaskContext()->rootTaskEP , sender);
            
            if( err == 0)
            {
                err = newProc->pid;
            }
        }
    }
    
    // We need to recreate the info message since it might have been modified in ProcessStart
    info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Spawn);
    seL4_SetMR(1 ,err);
    Reply( info);
}

void processWait(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    // 2nd reg is for returned pid;
    // 3rd reg is for returned status;
    
    int childCount = ProcessGetChildrenCount(sender);
    printf("[kernel_task] process %s wait on any child (%i child(ren))\n" , ProcessGetName(sender) , childCount);
    
    
    
    int err = -1;
    if( childCount == 0)
    {
        
        seL4_SetMR(0,SysCall_Wait);
        seL4_SetMR(1, -1 );
        seL4_SetMR(2, -1 );
        seL4_SetMR(3, -1 );
        Reply( info );
        return;
    }
    
    // does the process has any zombie children?
    Process* chld =  ProcessGetFirstChildZombie(sender);
    
    if( chld )//&& chld->status == ProcessState_Zombie)
    {
        assert(chld->status == ProcessState_Zombie);
        printf("[kernel_task]  got a zombie child %i\n" , chld->pid);
        seL4_SetMR(0, SysCall_Wait);
        seL4_SetMR(1, chld->pid);
        seL4_SetMR(2, chld->retCode);
        seL4_SetMR(3, chld->retSignal);
        ProcessCleanup(chld);
        Reply( info );
        
        
        return;
    }
    
    printf("[kernel_task]  NO zombie child we wait\n" );
    // if not, we wait
    
    assert(sender->reply == 0);
    assert(sender->replyState == ReplyState_None);
    
    sender->reply = get_free_slot(getVka());
    err = cnode_savecaller( getVka(), sender->reply );
    if( err != 0)
    {
        cnode_delete(getVka() , sender->reply);
        sender->reply = 0;
        
        seL4_SetMR(0,SysCall_Wait);
        seL4_SetMR(1, -1 );
        seL4_SetMR(2, -1 );
        Reply( info );
        return;
    }
    
    sender->replyState = ReplyState_Wait;
}

void processBootstrap(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    assert(sender->stats.numSysCalls == 0);
    seL4_SetMR(0, SysCall_BootStrap);
    //seL4_SetMR(1, (seL4_Word) sender->vaddr);
    seL4_SetMR(1, (seL4_Word) sender->venv);
    
    Reply( info);
}
