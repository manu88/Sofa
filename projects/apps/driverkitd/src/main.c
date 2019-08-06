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

#include <stdio.h>
#include <stdlib.h>
#include <sel4/sel4.h>
#include <assert.h>
#include <string.h>
#include <SysCalls.h>
#include <SysCaps.h>

int main( int argc , char* argv[])
{
    
    /*
    ps_io_mapper_t io_mapper;
    error =  sel4platsupport_new_io_mapper(context.vspace, context.vka, &io_mapper);
    assert(error == 0);
    */
    ServerEnvir* server =  RegisterServerWithName("driverkit", 0);

    if( server)
    {
        print("RegisterServerWithName OK\n");
        
        strcpy(server->buf , "Test Test 1-2");
    }
    else
    {
        print("RegisterServerWithName Error \n");
        return 1;
    }
    
    
    void* acpiAddr =  RequestResource( SofaResource_ACPI);
    
    if( acpiAddr)
    {
        print("RequestResource ACPI OK\n");
    }
    else
    {
        print("RequestResource ACPI error\n");
    }
    
	while(1)
	{
        int sender =  ServerRecv(server);
	}


	return 0;
}
