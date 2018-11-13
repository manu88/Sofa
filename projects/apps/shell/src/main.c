#include <SysClient.h>
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
#include <dirent.h>


int consoleFD  = -1;

static int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
    lenstr = strlen(str);
    
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

static int execCommand( char* cmd)
{
//	write(consoleFD, "\n" , 1);

//	if (strcmp(cmd , "ls") == 0)
	if (startsWith("ls ", cmd))
	{
		char* arg = cmd + strlen("ls ");
		printf("ls arg '%s' \n" , arg);
		
		struct dirent *dir;
    		DIR *d = opendir(arg);
		if (d)
    		{
        		while ((dir = readdir(d)) != NULL)
        		{
				write(consoleFD , dir->d_name , strlen(dir->d_name) );
				write(consoleFD , "\n" , 1);
            			printf("'%s'\n", dir->d_name);
        		}
        		closedir(d);
    		}


	}
	else if (strcmp(cmd , "pwd")  == 0)
	{
		char* pwd = getcwd(NULL, 0);

		write(consoleFD , pwd ,strlen(pwd));
		free(pwd);
	}
	else if (strcmp(cmd , "help") == 0)
	{
		const char b[] = "Some help you could use .... \n available command : ls \n";
    		write(consoleFD , b ,strlen(b));
	}
	else if (strcmp( cmd , "clear") == 0)
	{
		uint8_t msg[] = { 0xA , 0x0 , 0xB };
		write(consoleFD , msg , 3);
	}
	else if (startsWith("cd ", cmd))
	{
		char* arg = cmd + strlen("cd ");

		printf("Command ok arg : '%s'\n" , arg);
		int ret = chdir(arg);
		printf("chdir returned %i\n", ret);
	}
	else 
	{
		printf("unknown Command to exec : '%s' \n" , cmd);
	}
	return 0;
}

int main( int argc , char* argv[])
{
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }

    
    errno = 0;
    consoleFD = open("/dev/console" , O_RDWR);
    assert(consoleFD >=0);
    assert(errno == 0);

    const char b[] = "Sofa Shell - 2018";
    write(consoleFD , b ,strlen(b));

    char buf[4] = {0};


    char cmdBuf[128] = {0};
    size_t index = 0;

    write(consoleFD ,":>" , 2);

    while(1)
    {


        ssize_t readRet = read(consoleFD , buf , 4);

	if (readRet > 0)
	{
//    	    printf("readRet returned %i\n" , readRet);

	    for (int i=0;i<readRet ; i++)
	    {
	        if (buf[i] == '\n' || buf[i] == '\r')
	        {
		    cmdBuf[index++] = 0;
		    execCommand(cmdBuf);
//		printf("Typed command : '%s'\n",cmbBuf);
		
		    index = 0;
		    memset(&cmdBuf , 0 , 128);
		    write(consoleFD ,":>" , 2);

	        }
	        else 
	        {
	    	    cmdBuf[index++] = buf[i];
 	        }

//	        write(consoleFD ,&buf[i] , 1);
	    }
	}
    } // end while

    int appStatus = 0;
    pid_t childPid = wait(&appStatus);
    assert(childPid == -1);
    assert(errno == ECHILD);

/*
    errno = 0;
    int retPid = execve("app",NULL , NULL);
    printf("execve returned %i errno %i \n",retPid, errno);
    assert(errno == 0);
    assert(retPid > 1);
 
    errno = 0; 
    printf("Shell 1 : wait\n");
    childPid = wait(&appStatus);
    assert(childPid >= 1);

    printf("Start child again  /n");

    errno = 0; 
    retPid = execve("app",NULL , NULL);
    printf("execve returned %i errno %i \n",retPid, errno);
    assert(errno == 0);
    assert(retPid > 1);

    printf("Shell 2 : wait\n");
    childPid = wait(&appStatus);
    assert(childPid >= 1);
*/

//    printf("Wait returned %i status %i error %i\n",childPid , appStatus, errno);
    return 0;
}
