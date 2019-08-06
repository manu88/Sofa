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

#pragma once

#include <stdint.h>
#include "Sofa.h"
#include <SysCalls.h>
#include <utils/list.h>
#include <vka/object.h>
#include "KObject/KObject.h"

#define MAX_SERVER_NAME 128
#define MAX_CLIENT_NAME 128

typedef struct _Process Process; // forward

typedef struct _Server
{
    KSet base;
    //char name[MAX_SERVER_NAME];
    
    list_t clients;
    Process* owner;
    
    ServerEnvir* env;
    void *vaddr; // This is the shared addr of env
    
    
    vka_object_t srvTaskEP;
} Server;

typedef struct _Client
{
    KObject base;
    //char name[MAX_CLIENT_NAME];
    
    Process* owner;
    ClientEnvir* env;
    void *vaddr; // This is the shared addr of env
} Client;


int NameServerInitSystem(void);

Server* NameServerGetServerNamed(const char*name);

Server* NameServerRegister(Process*fromProcess, const char*name , int flags);

Client* NameServerCreateClient(Process*fromProcess , Server* toServer);

void NameServerRemoveAllFromProcess(Process*fromProcess);
int RemoveProcessAsClient( Process* fromProcess);

uint32_t NameServerGetNumClients(const Server* server);

void NameServerDump(void);
