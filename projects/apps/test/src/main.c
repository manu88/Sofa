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


int main( int argc , char* argv[])
{
	print("started has %i args\n" , argc);

    int f = 10 / 0;
    print("Result is %i\n" , f);
    
    ClientEnvir* client =  ConnectToServer( "driverkit");

    if (client)
    {
        print("Got client '%s'\n", client->buf);
        
        while (1)
        {

        }
    }
    else
    {
        print("Error client\n");
    }

	

	return 10;
}
