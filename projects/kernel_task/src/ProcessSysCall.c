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

static void processRegisterServer(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
static void processRegisterClient(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
static void processWait(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
static void processKill(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
static void processSleep(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
static void processSpawn(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
static void processExit(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);

#define Reply(i) seL4_Reply(i)
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
    
    switch(sysCallID)
    {
        case SysCall_BootStrap:
        {
            assert(sender->stats.numSysCalls == 0);
            seL4_SetMR(0, SysCall_BootStrap);
            seL4_SetMR(1, (seL4_Word) sender->vaddr);
            
            Reply( info);
        }
        case SysCall_Write:
        {
            printf("[%s] %s" ,ProcessGetName(sender), sender->env->buf);
            seL4_SetMR(1 , 0); // no err
            Reply( info);
            break;
        }
        case SysCall_Exit:
            processExit(sender,info ,sender_badge);
            break;
            
        case SysCall_Spawn:
            processSpawn(sender,info ,sender_badge);
            break;
            
        case SysCall_Kill:
            processKill(sender,info , sender_badge);
            break;
            
        case SysCall_Sleep:
            processSleep(sender,info , sender_badge);
            break;
        
        case SysCall_GetTime:
            seL4_SetMR(1 , GetCurrentTime());
            Reply( info);
            break;
        case SysCall_Read:
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
            
            sender->env->buf[0] = c;
            sender->env->buf[1] = 0;
            
            seL4_SetMR(0,SysCall_Read);
            seL4_SetMR(1, c >0? 1 : 0 );
            Reply( info );
            
        }
            break;
            
        case SysCall_Wait:
            processWait(sender,info , sender_badge);
            break;
        case SysCall_RegisterServer:
            processRegisterServer(sender,info , sender_badge);
            break;
        case SysCall_RegisterClient:
            processRegisterClient(sender,info , sender_badge);
            break;
            
        case SysCall_Debug:
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
                case SysCall_Debug_ListServers:
                    NameServerDump();
                    break;
                default:
                    assert(0);
                    break;
            }
        }
            break;
        default:
            assert(0);
    }
    
    sender->stats.numSysCalls++;
    
}

static void processExit(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    int retCode = seL4_GetMR(1);
    
    sender->retCode = retCode;
    printf("[kernel_task] program exited with code %i \n" , retCode);
    
    ProcessKill(sender);
    
    // nothing to return
}

static void processKill(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    int pidToKill = seL4_GetMR(1);
    printf("kill pid %i\n" , pidToKill);
    int err = -1;
    
    if( pidToKill > 1) // can't kill init or kernel_task
    {
        Process* procToKill = ProcessGetByPID(pidToKill);
        if (procToKill)
        {
            err = ProcessKill(procToKill);
        }
    }
    seL4_SetMR(0 , SysCall_Kill);
    seL4_SetMR(1 ,err);
    Reply( info);
}
static void processRegisterServer(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const char* serverName = sender->env->buf;
    
    int err = -1;
    
    void* ptr = NULL;
    if( serverName)
    {
        int flags = (int)seL4_GetMR(1);
        printf("[kernel_task] RegisterServer name '%s' with flags %i\n", sender->env->buf , flags );
        
        Server *serv = NameServerRegister(sender,serverName , flags);
        if( serv)
        {
            err = 0;
            ptr = serv->vaddr;
        }
        
    }
    
    seL4_SetMR(0,SysCall_RegisterServer);
    seL4_SetMR(1, err );
    seL4_SetMR(2, (seL4_Word) ptr );
    Reply( info );
}

static void processRegisterClient(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const char* serverName = sender->env->buf;
    
    printf("[kernel_task] RegisterClient to server '%s' from process '%s' %i \n", sender->env->buf , ProcessGetName(sender) , sender->pid );
    
    int err = -1;
    void* ptr = NULL;
    Server* server = NameServerGetServerNamed(serverName);
    if( server)
    {
        Client* clt = NameServerCreateClient(sender , server);
        
        if( clt)
        {
            printf("[kernel_task] client ok\n");
            err = 0;
            ptr = clt->vaddr;
        }
        else
        {
            printf("[kernel_task] client Error\n");
        }
    }
    else
    {
        printf("[kernel_task] server not found\n");
    }
    
    seL4_SetMR(0,SysCall_RegisterClient);
    seL4_SetMR(1, err );
    seL4_SetMR(2, (seL4_Word) ptr );
    Reply( info );
}

static void processSpawn(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    printf("[kernel_task] spawn '%s'\n", sender->env->buf);
    Process* newProc = malloc(sizeof(Process));
    
    int err = -1;
    
    if( newProc)
    {
        ProcessInit(newProc);
        err = ProcessStart(newProc, sender->env->buf ,&getKernelTaskContext()->rootTaskEP , sender);
        
        if( err == 0)
        {
            err = newProc->pid;
        }
    }
    
    // We need to recreate the info message since it might have been modified in ProcessStart
    info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Spawn);
    seL4_SetMR(1 ,err);
    Reply( info);
}
static void processWait(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
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
        Reply( info );
        return;
    }
    
    // does the process has any zombie children?
    Process* chld =  ProcessGetFirstChild(sender);
    
    if( chld && chld->status == ProcessState_Zombie)
    {
        printf("[kernel_task]  got a zombie child %i\n" , chld->pid);
        seL4_SetMR(0, SysCall_Wait);
        seL4_SetMR(1, chld->pid);
        seL4_SetMR(2, chld->retCode);
        
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


static int OnTime(uintptr_t token)
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

static void processSleep(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
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
