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

#include "ProcessSysCall.h"

#include <platsupport/chardev.h>

#include "Timer.h"
#include "Bootstrap.h"
#include "Utils.h"
#include "NameServer.h"






// order MUST Match SysCallID
static SysCallHandler _sysHandler[SysCall_Last] =
{
    NULL, // 0 is UNUSED for now
    processBootstrap,
    processGetIDs,
    processWrite,
    processSleep,
    processRead,
    processDebug,
    processSpawn,
    processKill,
    processWait,
    processExit,
    processGetTime,
    processSetPriority,
    processGetPriority,
    processCapOp,
    processRegisterServer,
    processRegisterClient,
};


/*
static inline void Reply(seL4_MessageInfo_t info)
{
    printf("Reply syscall %li\n" , seL4_GetMR(0));
    seL4_Reply(info);
}
 */




void processSysCall(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    assert(sender);
    const SysCallID sysCallID = seL4_GetMR(0);
    
    _sysHandler[sysCallID](sender,info ,sender_badge);
    
    sender->stats.numSysCalls++;
}


void processGetTime(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    seL4_SetMR(1 , GetCurrentTime());
    Reply( info);
}



void processWrite(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    printf("[%s] %s" ,ProcessGetName(sender), sender->bufEnv->buf);
    seL4_SetMR(1 , 0); // no err
    Reply( info);
}

void processRead(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    unsigned long sizeToRead = seL4_GetMR(1);
    
    
    int c = ps_cdev_getchar(&getKernelTaskContext()->comDev);
    /*
     do
     {
     c = ps_cdev_getchar(&getKernelTaskContext()->comDev);
     }
     while (c<=0);
     */
    if( c == '\n' || c == '\r')
    {
        ps_cdev_putchar(&getKernelTaskContext()->comDev , '\n');
    }
    else if( c > 0)
    {
        ps_cdev_putchar(&getKernelTaskContext()->comDev , c);
    }
    
    sender->bufEnv->buf[0] = c;
    sender->bufEnv->buf[1] = 0;
    
    seL4_SetMR(0,SysCall_Read);
    seL4_SetMR(1, c >0? 1 : 0 );
    Reply( info );
}

void processBootstrap(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    assert(sender->stats.numSysCalls == 0);
    seL4_SetMR(0, SysCall_BootStrap);
    seL4_SetMR(1, (seL4_Word) sender->vaddr);
    
    Reply( info);
}

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



int OnTime(uintptr_t token)
{
    Process* sender = (Process*) token;
    //seL4_CPtr reply = (seL4_CPtr) token;
    if( !sender->reply)
    {
        printf("OnTime : no reply for %s %i\n" , ProcessGetName(sender) , sender->pid);
    }
    assert(sender->reply);
    assert(sender->replyState == ReplyState_Sleep);
    
    int err = tm_deregister_cb(getTM()  , sender->timerID);
    assert(err == 0);
    
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, SysCall_Sleep);
    seL4_SetMR(1, 0); // sucess
    
    seL4_Send(sender->reply , tag);
    
    cnode_delete(getVka(),sender->reply);
    sender->reply = 0;
    sender->replyState = ReplyState_None;
    return 0;
}

void processSleep(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    if( sender->timerID == 0)
    {
        int err = tm_alloc_id(getTM() , &sender->timerID);
        if( err != 0)
        {
            seL4_SetMR(0,SysCall_Sleep);
            seL4_SetMR(1, -EPERM );
            Reply( info );
            
        }
    }
    assert(sender->timerID);
    
    if( sender->reply != 0)
    {
        printf("sender->reply not null for %s %i\n" , ProcessGetName(sender) , sender->pid);
    }
    assert(sender->reply == 0);
    int err = -1;
    
    uint64_t ns = seL4_GetMR(1);
    SleepUnit unit = seL4_GetMR(2);
    
    
    if( unit == SleepUnit_MS)
    {
        ns *= 1000000;
    }
    else if( unit == SleepUnit_S)
    {
        ns *= 1000000000;
    }
    sender->reply = get_free_slot(getVka());
    
    if( sender->reply == 0)
    {
        seL4_SetMR(0,SysCall_Sleep);
        seL4_SetMR(1, -EINVAL );
        Reply( info );
        return;
    }
    
    err = cnode_savecaller( getVka(), sender->reply );
    if( err != 0)
    {
        cnode_delete(getVka() , sender->reply);
        sender->reply = 0;
        
        seL4_SetMR(0,SysCall_Sleep);
        seL4_SetMR(1, -EINVAL );
        Reply( info );
        return;
    }
    
    sender->replyState = ReplyState_Sleep;
    err = tm_register_rel_cb( getTM() , ns , sender->timerID , OnTime ,(uintptr_t) sender);

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
