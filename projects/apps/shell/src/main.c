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

#ifndef __APPLE__
#include <SysClient.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


#include "BaseCommands.h"


static int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
    lenstr = strlen(str);
    
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

static int execCommand( char* cmd)
{
	if (startsWith("ls", cmd))
	{
		char* arg = cmd + strlen("ls ");
        if ( strlen(arg) == 0 )
        {
            arg = ".";
        }
        
        return exec_ls(arg);
	}
    else if (startsWith("exec", cmd))
    {
        char* arg = cmd + strlen("exec ");
        
        return exec_exec(arg);
    }
    else if (startsWith("cat", cmd))
    {
        char* arg = cmd + strlen("cat ");

        return exec_cat(arg);
    }
    else if (startsWith("mkdir", cmd))
    {
        char* arg = cmd + strlen("mkdir ");

        return exec_mkdir(arg);
    }
    else if (startsWith("touch", cmd))
    {
        char* arg = cmd + strlen("touch ");
        
        return exec_touch(arg);
    }
    else if (startsWith("kill", cmd))
    {
        char* arg = cmd + strlen("kill ");
        
        return exec_kill(arg);
    }
    else if (startsWith("sleep", cmd))
    {
        char* arg = cmd + strlen("sleep ");
        
        return exec_sleep(arg);
    }
    else if (startsWith("ps", cmd))
    {
        char* arg = cmd + strlen("ps ");
        
        return exec_ps(arg);
    }
    else if (strcmp(cmd , "whoami")  == 0)
    {
        const char b[] = "god";
        writeConsole( b  ,strlen(b));
        return 0;
    }
	else if (strcmp(cmd , "pwd")  == 0)
	{
		char* pwd = getcwd(NULL, 0);

		writeConsole(  pwd ,strlen(pwd));
		free(pwd);
        
        return 0;
	}
	else if (strcmp(cmd , "help") == 0)
	{
        PrintHelp();
        return 0;
	}
	else if (strcmp( cmd , "clear") == 0)
	{
		uint8_t msg[] = { 0xA , 0x0 , 0xB };
		writeConsole(  msg , 3);
        return 0;
	}
	else if (startsWith("cd ", cmd))
	{
		char* arg = cmd + strlen("cd ");
		int ret = chdir(arg);

		if (ret != 0 && errno == ENOTDIR)
		{
			const char b[] = "Not a directory";
        		writeConsole( b  ,strlen(b));
		}
		return ret;

	}
    else if (strcmp(cmd, "exit") == 0)
    {
        exit(0);
    }
	else if( strlen(cmd))
	{
		const char msg[] = "unknown command ";
		writeConsole(msg , strlen(msg) );
		writeConsole(cmd , strlen(cmd) );
		
	}

    
	return 0;
}

int main( int argc , char* argv[])
{
#ifndef __APPLE__
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }
    
    errno = 0;
    int consoleFD = open("/dev/console" , O_RDWR);
    assert(consoleFD >=0);
    assert(errno == 0);
    
#endif
    
#ifdef __APPLE__
    InitConsoleFDs(STDIN_FILENO, STDOUT_FILENO);
#else
    InitConsoleFDs(consoleFD, consoleFD);
#endif
    
    setTermColor(VGA_COLOR_RED);
    const char b[] = "Sofa Shell - 2018\n";
    writeConsole(  b ,strlen(b));

    char buf[4] = {0};


    char cmdBuf[128] = {0};
    size_t index = 0;
    setTermColor(VGA_COLOR_GREEN);

    writeConsole( ":>" , 2);
    setTermColor(VGA_COLOR_WHITE);
    while(1)
    {
        ssize_t readRet = readConsole( buf , 4);

        if (readRet > 0)
        {


            for (int i=0;i<readRet ; i++)
            {
                if (buf[i] == '\n' || buf[i] == '\r')
                {
                    cmdBuf[index++] = 0;
                    if(execCommand(cmdBuf))
                    {
                        writeConsole("\n" , 1);
                    }

		
                    index = 0;
                    memset(&cmdBuf , 0 , 128);
                   setTermColor(VGA_COLOR_GREEN); 
	   	   writeConsole( ":>" , 2);
			setTermColor(VGA_COLOR_WHITE);

                }
                else
                {
                    cmdBuf[index++] = buf[i];
                }

            }
        }
    
    } // end while

    return 0;
}
