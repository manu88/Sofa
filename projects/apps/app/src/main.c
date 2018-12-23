#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SysClient.h>

#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>

#include<signal.h>

static int consoleFDWRead  = 0;
static int consoleFDWrite  = 0;

ssize_t writeConsole( const void* b , size_t len)
{
    return write(consoleFDWrite, b, len);
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

void sig_handler(int signo)
{
  if (signo == SIGINT)
    printf("received SIGINT\n");
}


int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }
/*
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
	printf("\ncan't catch SIGINT\n");
    }
*/
    clearTerm();

//    for(int i= 0;i<4;i++)
    
    //while(1)
    //{
	//sleep(1);
	//setTermCoords(
    //}

    for(uint8_t y = 0; y<5/*25*/;++y)
    {
	setTermColor(y+1);

        for(uint8_t x = 0; x<80;++x)
	{
		setTermCoords(x ,y);
		writeConsole("|" , 1);
	}

    }

    clearTerm();
/*
    int ret = usleep(1000*4000);
    assert(ret == 0);
    assert(errno == 0);


    printf("Client After sleep\n");
*/
    return 10;
}

