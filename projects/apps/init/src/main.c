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
#include <assert.h>
#include <string.h>
#include <SysCalls.h>


int main( int argc , char* argv[])
{


	print("started\n");

    int pidShell = -1;
    int pidDriverKit = -1;
    pidDriverKit = spawn("driverkitd",0 , NULL);
    
    print("spawn driverkitd pid %i\n"  , pidDriverKit);
    
    

	pidShell = spawn("shell",0 , NULL);

	print("spawn shell pid %i\n"  , pidShell);

	while(1)
	{
        SofaSignal signal = 0;
        int wstatus = 0;
        long retPID = wait(&wstatus ,&signal);
        
        print("PID %ld returned with status %i signal %i\n" , retPID  , wstatus , signal);
        
        if( retPID == pidShell)
        {
            print("Restart Shell\n");
            pidShell = spawn("shell",0 , NULL);
        }
	}


	return 0;
}
