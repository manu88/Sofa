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
#include "../Timer.h"
#include "../Bootstrap.h"
#include "../kUtils.h"

void processGetTime(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    seL4_SetMR(1 , GetCurrentTime());
    Reply( info);
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

