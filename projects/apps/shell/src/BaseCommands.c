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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include "BaseCommands.h"
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stddef.h>
#include <assert.h>


static int consoleFDWrite  = -1;
static int consoleFDWRead  = -1;

int InitConsoleFDs(int fdRead , int fdWrite)
{
    consoleFDWRead = fdRead;
    consoleFDWrite = fdWrite;
    return 1;
}


ssize_t writeConsole( const void* b , size_t len)
{
    return write(consoleFDWrite, b, len);
}


ssize_t readConsole( void*b , size_t len)
{
    return read(consoleFDWRead, b, len);
}

void setTermColor( int color)
{
	char b[12] = {0};
	ssize_t ret = snprintf(b , 12 , "\033[%i;40;0m" , color + 30);
	writeConsole(b , ret);
}

void clearTerm()
{
    static const char cmd[] = "\033[2J\033[0;0H";
    writeConsole(  cmd , sizeof(cmd));
}

void setTermCoords(uint8_t x , uint8_t y)
{
    char b[12] = {0};
    ssize_t ret = snprintf(b , 12 , "\033[%u;%uH" , y,x);
    writeConsole(b , ret);
}

void PrintHelp()
{
    static const char b[] = "available command :\n"
    "ls exec cat mkdir touch kill sleep rm ps stat pwd help clear renice cd exit\n";
    
    writeConsole(  b ,strlen(b));
}

int exec_ls( const char* args)
{
    struct dirent *dir;
    DIR *d = opendir(args);
    if (d)
    {
        char p[1024] = {0};
        
        while ((dir = readdir(d)) != NULL)
        {
            ssize_t retS = snprintf(p , 1024 , "%s/%s",args,dir->d_name);

            struct stat _stat;
            int ret = stat(p , &_stat);

            assert(ret == 0);


            writeConsole( dir->d_name , strlen(dir->d_name) );

            retS = snprintf(p  , 1024 , " %u " , _stat.st_uid);
            writeConsole(p , retS);

            writeConsole( "\n" , 1);
        }
        
        closedir(d);
        return 0;
        
    }
    
    return errno;
    
}

int exec_cat( const char* args)
{
    int f = open(args, O_RDONLY);
    
    if (f<0)
    {
        return f;
    }
    
    char b[32] = {0};
    
    ssize_t r = 0;
    do
    {
        r = read(f, b, 32);
        writeConsole(b, r);
    } while (r > 0);
    
    close(f);
    
    return 0;
}


int exec_touch( const char* args)
{
    int f = open(args, O_WRONLY | O_APPEND | O_CREAT , 0644);
    
    if (f >=0)
    {
        close(f);
        return 1;
    }
    return 0;
}

int exec_exec( const char* args)
{
    char shouldWaitChar = 0;
    char appName[128] = {0};
    int n = sscanf(args, "%s %c" , appName , &shouldWaitChar);
    
    int shouldWait = shouldWaitChar == 0;
    if (n > 0)
    {
        
        int retPid = execve(appName,NULL , NULL);
        
        if (shouldWait)
        {
            int status = 0;
            waitpid(retPid, &status , 0);
        }
        
        return retPid;
    }
    return 0;
}


int exec_ps( const char* args)
{
    struct dirent *dir;
    DIR *d = opendir("/proc/");
    
    if (!d)
    {
        return -1;
    }

    static const char strHeader[] = "USER PID TTY          TIME CMD";
    writeConsole(strHeader , strlen(strHeader));
    writeConsole("\n", 1);

    static char b[1024] = {0};
    static char path[1024] = {0};
    static char strUid[5] ={0};
    
    while ((dir = readdir(d)) != NULL)
    {
	
        memset(b, 0, 1024);
        memset(path, 0, 1024);
        memset(strUid , 0 , 5);

        // get folder stat
        struct stat st;

        snprintf( path , 1024 , "/proc/%s/" , dir->d_name);
        int ret = stat(path , &st);
        assert(ret == 0);
        //
        snprintf( path , 1024 , "/proc/%s/cmdline" , dir->d_name);
        //printf("cmd line path is '%s'\n" , path);
	
        //write the user
        uid_t userId = st.st_uid;
	
        size_t sizeUid = snprintf( strUid ,5, "%u" , userId);
        writeConsole(strUid,  sizeUid);

        // some pad
        for(int i=0;i<5-sizeUid;++i)
        {
            writeConsole(" ",1);
        }
        
        // write the PID
        size_t s = snprintf(b, 1024, "%s                     ", dir->d_name);
        writeConsole(b , s);

        int f = open(path, O_RDONLY);// fopen(path , "r");
        
        if(f >= 0)
        {
            char buf[32] = {0};

    	    ssize_t r = 0;
            do 
            {
                r = read(f, buf, 32);

                writeConsole(buf, r);
            } while (r > 0);
    	    
            close(f);

        }
        else
        {
            printf("Error fopen '%s'\n",path);
        }
        
        writeConsole("\n", 1);

    }
    
    closedir(d);
    
    
    return 0;
}

int exec_kill( const char* args)
{
	long pidToKill = atol(args);

	return kill( (pid_t)pidToKill , SIGTERM);
}


int exec_mkdir( const char* args)
{
	return mkdir(args , 1);
}


int exec_sleep(const char* args)
{
	struct timespec ts;
	ts.tv_sec = atoi(args);
	ts.tv_nsec = 0;


	return nanosleep(&ts , NULL);

}

int exec_stat(const char* args)
{
	struct stat s;

	int ret = stat(args , &s);
#ifndef __APPLE__
	printf("Created at %li seconds %li ns\n" , s.st_mtim.tv_sec,s.st_mtim.tv_nsec);
#endif
	printf("file uid %u\n" , s.st_uid);
	return ret;
}


int exec_renice( const char* args)
{
    int who = 0;
    int niceVal = 0;
    int n = sscanf(args, "%i %i" , &who , &niceVal);
    
    if (n == 2)
    {
        printf("renice pid %i value %i\n" , who , niceVal);
        
        return setpriority(PRIO_PROCESS, who, niceVal);
    }
    else
    {
        return -EINVAL;
    }
    
    return 0;
}

int exec_echo( const char* args)
{
    if (strcmp(args , "$$" ) == 0)
    {
	const pid_t pid = getpid();
	char str[16] = "";
	snprintf(str , 16 , "%i" , pid);

	writeConsole(str, strlen(str) );
    }
    else
    {
        writeConsole(args, strlen(args));
    }
    return 0;
}


int exec_write( const char* args)
{
	printf("Write args '%s'\n" , args);
	return 0;
}
