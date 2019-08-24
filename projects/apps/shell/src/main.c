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
    else if( startsWith("run " , cmd))
    {
        const char* arg = cmd + strlen("run ");
        if( !arg || strlen(arg ) == 0)
        {
            return -1;
        }
        
        int retPid = spawn(arg ,0 , NULL);
        if( retPid <=  0)
        {
            return retPid;
        }
        
        SofaSignal signal = 0;
        int status = -1;
        long pid = wait(&status ,&signal);
        print("PID %li returned %i sig %i\n" , pid , status , signal);
        return 0;
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
    else if( strcmp("exit" , cmd) == 0)
    {
        doExit(0);
    }
    else if( strcmp("wait" , cmd) == 0)
    {
        SofaSignal signal = 0;
        int status = -1;
        long pid = wait(&status ,&signal);
        
        if( pid == -1)
        {
            print("No child to wait on \n");
            return -1;
        }
        print("PID %li returned %i sig %i\n" , pid , status , signal);
        return 0;
    }
    else if( strcmp("time" , cmd) == 0)
    {
        uint64_t timeNS =  getTime();

        uint64_t secs = timeNS / 1000000000;
        uint64_t remainsNS = timeNS - (secs * 1000000000);
        uint64_t ms = remainsNS / 1000000;

        print("Time %li:%li ms\n" , secs , ms );

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
    else if( startsWith("setprio ", cmd))
    {
        const char* arg = cmd + strlen("setprio ");
        
        int pid = 0;
        int prio = 0;
        
        if( sscanf(arg , "%i %i" , &pid , &prio) == 2)
        {
            return setPriority(pid ,prio);
        }
    }
    else if( startsWith("getprio ", cmd))
    {
        const char* arg = cmd + strlen("getprio ");
        
        int pid = 0;
        
        if( sscanf(arg , "%i" , &pid ) == 1)
        {
            int retPrio  = 0;
            int ret =  getPriority(pid ,&retPrio);
            
            if( ret == 0)
            {
                print("Priority  : %i\n" , retPrio);
            }
            return ret;
        }
    }
    else if( startsWith("setcap ", cmd))
    {
        const char* arg = cmd + strlen("setcap ");
        
        int cap = 0;
        
        if( sscanf(arg , "%i" , &cap ) == 1)
        {
            
            return CapAcquire(cap);
        }
    }
    else if( startsWith("dropcap ", cmd))
    {
        const char* arg = cmd + strlen("dropcap ");
        
        int cap = 0;
        
        if( sscanf(arg , "%i" , &cap ) == 1)
        {
            CapDrop(cap);
            return 0;
        }
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
