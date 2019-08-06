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

#include <assert.h>
#include <allocman/vka.h>

#include "../ProcessSysCall.h"
#include "../NameServer.h"
#include "../Bootstrap.h"


void processRegisterServer(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const char* serverName = ProcessGetIPCBuffer(sender);
    
    int err = -1;
    
    void* ptr = NULL;
    seL4_CPtr movedEP = 0;
    if( serverName)
    {
        int flags = (int)seL4_GetMR(1);
        printf("[kernel_task] RegisterServer name '%s' with flags %i\n", ProcessGetIPCBuffer(sender) , flags );
        
        Server *serv = NameServerRegister(sender,serverName , flags);
        if( serv)
        {
            err = 0;
            ptr = serv->vaddr;
        }
#if 0
/* 1. Create an endpoint for the server*/
        int error = vka_alloc_endpoint( getVka(), &serv->srvTaskEP);
        if( error != 0)
        {
            printf("vka_alloc_endpoint for Server's endpoint failed %i\n" , error);
            assert(0);
        }
        
        cspacepath_t ep_cap_path;
        vka_cspace_make_path( getVka(), serv->srvTaskEP.cptr, &ep_cap_path);
        
/* Create a minted copy*/
        
        cspacepath_t ep_cap_path;
        vka_cspace_make_path( getVka(), server->srvTaskEP.cptr, &ep_cap_path);
        seL4_CPtr mintedEP =  sel4utils_mint_cap_to_process(&sender->native, ep_cap_path, seL4_AllRights, sender->pid);
        
        if( mintedEP == 0)
        {
            printf("mintedEP is NULL\n");
        }
        assert(mintedEP);
        
/* 2. move it to server's process*/
        movedEP =  sel4utils_move_cap_to_process(&sender->native, ep_cap_path, getVka() );
        
        if( movedEP == 0)
        {
            printf("movedEP is NULL\n");
        }
        assert(movedEP);
#endif
    }

    seL4_SetMR(0,SysCall_RegisterServer);
    seL4_SetMR(1, err );
    seL4_SetMR(2, (seL4_Word) ptr );
    seL4_SetMR(3, (seL4_Word) movedEP );
    Reply( info );
}

void processRegisterClient(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    const char* serverName = ProcessGetIPCBuffer(sender);
    
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
