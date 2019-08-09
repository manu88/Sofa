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

#include "NameServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/list.h>

#include "Bootstrap.h"
#include "Process.h"
#include "Utils.h"
static list_t _servers = {0};

int NameServerInitSystem()
{
    int ret = -1;
    
    ret = list_init(&_servers);
    if( ret != 0)
    {
        return ret;
    }
    
    return ret;
    
}

Server* NameServerGetServerNamed(const char*name)
{
    for (struct list_node *n = _servers.head; n != NULL; n = n->next)
    {
        
        Server* server = n->data;
        
        if( strcmp(KObjectGetName( (const KObject*) server) , name) == 0)
        {
            return server;
        }
    }
    
    return NULL;
}


static Server* NameServerGetFirstFromProcess( Process*fromProcess)
{
    for (struct list_node *n = _servers.head; n != NULL; n = n->next)
    {
        Server* server = n->data;
        if( server->owner == fromProcess)
        {
            return server;
        }
    }
    
    return NULL;
}

static int ServComp(void*a, void*b)
{
    Server* s1 = a;
    Server* s2 = b;
    
    if( s1 == s2)
    {
        return 0; // if equals
    }
    return -1;
}

static int CltComp(void*a, void*b)
{
    Client* s1 = a;
    Client* s2 = b;
    
    if( s1 == s2)
    {
        return 0; // if equals
    }
    return -1;
}

static int NameServerRemove(Server* server)
{
    return list_remove(&_servers , server ,ServComp );
}

static Client* NameServerGetFirstClientFromProcess(Server* server, Process*fromProcess)
{
    for (struct list_node *n = server->clients.head; n != NULL; n = n->next)
    {
        Client* clt = n->data;
        if( clt->owner == fromProcess)
        {
            return clt;
        }
    }
    
    return NULL;
}

int RemoveProcessAsClient( Process* fromProcess)
{
    for (struct list_node *n = _servers.head; n != NULL; n = n->next)
    {
        Server* server = n->data;
        
        Client* clt = NameServerGetFirstClientFromProcess(server, fromProcess);
        if( clt)
        {
            return list_remove(&server->clients , clt ,CltComp );
        }
    }
    
    return -1;
}

void NameServerRemoveAllFromProcess(Process*fromProcess)
{
    Server* s = NULL;
    
    while ((s = NameServerGetFirstFromProcess(fromProcess) ))
    {
        NameServerRemove(s);
    }

}



static seL4_CPtr initEndPoint(Process* fromProcess)
{
    vka_object_t ep;
    int error = vka_alloc_endpoint( getVka(), &ep);
    if( error != 0)
    {
        printf("vka_alloc_endpoint for NameServer failed %i\n" , error);
        
        return 0;
    }
    
    cspacepath_t ep_cap_path;
    vka_cspace_make_path( getVka(), ep.cptr, &ep_cap_path);
    
    return ep.cptr;
}

static int NameServerInit(Server* s, const char*name)
{
    memset(s, 0 , sizeof(Server));
    
    KSetInitWithName(&s->base ,strdup( name));
 
    int ret = list_init(&s->clients);
    return ret;
}

static void ServerRelease(KObject* obj)
{
    
}

uint32_t NameServerGetNumClients(const Server* server)
{
    return list_length((list_t*)&server->clients);
}

Server* NameServerRegister(Process*fromProcess, const char*name , int flags)
{
    assert(fromProcess);
    assert(name);
    if( NameServerGetServerNamed(name) != NULL)
    {
        return NULL;
    }
    
    Server* server = malloc(sizeof(Server));
    if( !server)
    {
        return NULL;
    }
    
    if( NameServerInit(server ,name) != 0)
    {
        free(server);
        return NULL;
    }
    //strncpy(server->name , name,MAX_SERVER_NAME);
    
    
    /* Shared mem */
    size_t numPages = 1;
    server->env = vspace_new_pages(getVspace(),seL4_AllRights , numPages , PAGE_BITS_4K);
    if( server->env == NULL)
    {
        printf("Error vspace_new_pages\n");
        free(server);
        return NULL;
    }
    
    seL4_CPtr process_data_frame = vspace_get_cap(getVspace(), server->env);
    if( process_data_frame == 0)
    {
        printf("vspace_get_cap failed\n");
        free(server);
        return NULL;
    }
    
    seL4_CPtr process_data_frame_copy = 0;
    sel4osapi_util_copy_cap(getVka(), process_data_frame, &process_data_frame_copy);
    if( process_data_frame_copy == 0)
    {
        printf("sel4osapi_util_copy_cap failed\n");
        free(server);
        return NULL;
    }
    
    
    server->env->endpoint = initEndPoint(fromProcess);
    assert( server->env->endpoint != 0);
    
    server->vaddr = vspace_map_pages(&fromProcess->native.vspace, &process_data_frame_copy, NULL, seL4_AllRights, numPages, PAGE_BITS_4K, 1/*cacheable*/);
    if( server->vaddr == NULL)
    {
        printf("vspace_map_pages error \n");
        free(server);
        return NULL;
    }
    assert(server->vaddr != 0);
    
    server->owner = fromProcess;
    
    if(list_prepend(&_servers , server) == 0)
    {
        return server;
    }
    free(server);
    return NULL;
    
}

void NameServerDump()
{
    for (struct list_node *n = _servers.head; n != NULL; n = n->next)
    {
        Server* server = n->data;
        
        printf("Server %s %i client(s)\n" , KObjectGetName( (const KObject*) server) , NameServerGetNumClients( server));
    }
}

int ClientInit(Client *client)
{
    memset(client , 0 , sizeof(Client));
    
    KObjectInit(&client->base);
    return 0;
}

Client* NameServerCreateClient(Process*fromProcess , Server* toServer)
{
    Client* clt = malloc(sizeof(Client));
    if(!clt)
    {
        return NULL;
    }
    
    if(ClientInit(clt) != 0 )
    {
        return NULL;
    }
    
    /* Shared mem */
    size_t numPages = 1;
    clt->env = toServer->env;// = vspace_new_pages(getVspace(),seL4_AllRights , numPages , PAGE_BITS_4K);
    if( clt->env == NULL)
    {
        printf("Error vspace_new_pages\n");
        free(clt);
        return NULL;
    }
    
    seL4_CPtr process_data_frame = vspace_get_cap(getVspace(), clt->env);
    if( process_data_frame == 0)
    {
        printf("vspace_get_cap failed\n");
        free(clt);
        return NULL;
    }
    
    seL4_CPtr process_data_frame_copy = 0;
    sel4osapi_util_copy_cap(getVka(), process_data_frame, &process_data_frame_copy);
    if( process_data_frame_copy == 0)
    {
        printf("sel4osapi_util_copy_cap failed\n");
        free(clt);
        return NULL;
    }
    
    
    //clt->env->endpoint = initEndPoint(fromProcess);
    //assert( server->env->endpoint != 0);
    
    clt->vaddr = vspace_map_pages(&fromProcess->native.vspace, &process_data_frame_copy, NULL, seL4_AllRights, numPages, PAGE_BITS_4K, 1/*cacheable*/);
    if( clt->vaddr == NULL)
    {
        printf("vspace_map_pages error \n");
        free(clt);
        return NULL;
    }
    assert(clt->vaddr != 0);
    
    if(list_prepend(&toServer->clients , clt) == 0)
    {
        clt->owner = fromProcess;
        return clt;
    }
    
    printf("Error adding client to server\n");
    return NULL;
}
