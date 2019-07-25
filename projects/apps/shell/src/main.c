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


static int execCmd( const char* cmd);

int main( int argc , char* argv[])
{
	//int ret = InitClient(argc , argv);



	print("started\n");

    char cmd[128] = "";
    int cmdPos = 0;
    
	while(1)
	{


		char buf[2];
		long didRead = read( buf, 2);
		if( didRead > 0)
		{
			if( buf[0] == '\n' || buf[0] == '\r')
            {
                print("\n");
                int ret = execCmd(cmd);
                
                print("'%s' returned %i\n" , cmd,ret);

                
                //clean
                memset(cmd, 0, 128);
                cmdPos = 0;
            }
            else
            {
                cmd[cmdPos++] = buf[0];
            }
		}

	}


	return 0;
}

static int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
    lenstr = strlen(str);
    
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}


static int execCmd( const char* cmd)
{
    
    if( startsWith("ps", cmd))
    {
        ps();
    }
    else if( startsWith("sched", cmd))
    {
        sched();
    }
    else if( startsWith("spawn ", cmd))
    {
        const char* arg = cmd + strlen("spawn ");
        if( !arg || strlen(arg ) == 0)
        {
            return -1;
        }
        
        return spawn(arg ,0 , NULL);
    }
    else if( startsWith("kill ", cmd))
    {
        const char* arg = cmd + strlen("kill ");
        int which = atoi(arg);
        
        if( which > 0)
        {
            return kill(which);
        }
        
        return -1;
    }
    else if( strcmp("wait" , cmd) == 0)
    {
        int status = -1;
        long pid = wait(&status);
        print("PID %li returned %i\n" , pid , status);
        return 0;
    }
    else if( strcmp("time" , cmd) == 0)
    {
        print("Time %li\n" , getTime() );
        return 0;
    }
    else if( startsWith("sleep ", cmd))
    {
        const char* arg = cmd + strlen("sleep ");
        if( !arg || strlen(arg ) == 0)
        {
            return -1;
        }
        
        long ms = atol(arg);
        
        if( ms <= 0)
        {
            return -1;
        }
        
        return sleepMS(ms);
    }
    else if( strcmp("servers" , cmd) == 0)
    {
        listServers();
    }
    else if( strcmp("fault" , cmd) == 0)
    {
        int*lol = NULL;
        *lol = 4;
    }
    else
    {
        print("unknown command '%s'\n" , cmd);
        return -1;
    }
    return 0;
}
