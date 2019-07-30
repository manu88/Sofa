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
#include "../NameServer.h"

void processRegisterServer(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const char* serverName = sender->bufEnv->buf;
    
    int err = -1;
    
    void* ptr = NULL;
    if( serverName)
    {
        int flags = (int)seL4_GetMR(1);
        printf("[kernel_task] RegisterServer name '%s' with flags %i\n", sender->bufEnv->buf , flags );
        
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

void processRegisterClient(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const char* serverName = sender->bufEnv->buf;
    
    printf("[kernel_task] RegisterClient to server '%s' from process '%s' %i \n", serverName , ProcessGetName(sender) , sender->pid );
    
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
