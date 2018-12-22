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

static int consoleFDWRead  = 0;
static int consoleFDWrite  = 0;

ssize_t writeConsole( const void* b , size_t len)
{
    return write(consoleFDWrite, b, len);
}


void setTermColor( int color)
{
#ifndef __APPLE__
 	uint8_t msg[] = { 0xA , 0x2 , color };
    writeConsole(  msg , 3);
#endif
}

void clearTerm()
{
    uint8_t msg[] = { 0xA , 0x0 , 0xB };
    writeConsole(  msg , 3);
}

void setTermCoords(uint8_t x , uint8_t y)
{
    uint8_t msg[] = { 0xA , 0x3 , x , y };
    writeConsole(  msg , 4);
}

int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }

    clearTerm();

//    for(int i= 0;i<4;i++)
    
    //while(1)
    //{
	//sleep(1);
	//setTermCoords(
    //}

    for(uint8_t y = 0; y<25;++y)
    {
	setTermColor(y+1);
        for(uint8_t x = 0; x<80;++x)
	{
		setTermCoords(x ,y);
		writeConsole("|" , 1);
	}

    }
/*
    int ret = usleep(1000*4000);
    assert(ret == 0);
    assert(errno == 0);


    printf("Client After sleep\n");
*/
    return 10;
}

