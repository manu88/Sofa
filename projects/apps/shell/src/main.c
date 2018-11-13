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
#include <dirent.h>

#ifndef __APPLE__
static int consoleFD  = -1;
#endif

static ssize_t writeConsole( const void* b , size_t len)
{
#ifndef __APPLE__
    return write(consoleFD, b, len);
#else
    return write(STDOUT_FILENO, b, len);
#endif
}


static ssize_t readConsole( void*b , size_t len)
{
#ifndef __APPLE__
    return read(consoleFD, b, len);
#else
    return read(STDIN_FILENO, b, len);
#endif
}

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
	if (startsWith("ls", cmd))
	{
		char* arg = cmd + strlen("ls ");
        
        if ( strlen(arg) == 0 )
        {
            arg = ".";
        }
		printf("ls arg '%s' \n" , arg);
		
		struct dirent *dir;
    		DIR *d = opendir(arg);
		if (d)
    		{
        		while ((dir = readdir(d)) != NULL)
        		{
				writeConsole( dir->d_name , strlen(dir->d_name) );
				writeConsole( "\n" , 1);
            			printf("'%s'\n", dir->d_name);
        		}
        		closedir(d);
    		}


	}
	else if (strcmp(cmd , "pwd")  == 0)
	{
		char* pwd = getcwd(NULL, 0);

		writeConsole(  pwd ,strlen(pwd));
		free(pwd);
	}
	else if (strcmp(cmd , "help") == 0)
	{
		const char b[] = "Some help you could use .... \n available command : ls \n";
    		writeConsole(  b ,strlen(b));
	}
	else if (strcmp( cmd , "clear") == 0)
	{
		uint8_t msg[] = { 0xA , 0x0 , 0xB };
		writeConsole(  msg , 3);
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
#ifndef __APPLE__
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }



    
    errno = 0;
    consoleFD = open("/dev/console" , O_RDWR);
    assert(consoleFD >=0);
    assert(errno == 0);
    
#endif

    const char b[] = "Sofa Shell - 2018";
    writeConsole(  b ,strlen(b));

    char buf[4] = {0};


    char cmdBuf[128] = {0};
    size_t index = 0;

    writeConsole( ":>" , 2);

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
                    execCommand(cmdBuf);
		
                    index = 0;
                    memset(&cmdBuf , 0 , 128);
                    writeConsole( ":>" , 2);

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
