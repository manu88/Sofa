#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SysClient.h>

int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
	return 1;
    }

    printf("Client : Hello\n");

    pid_t pid = getpid();
    pid_t parentPid = getppid();
    printf("Client PID is %i parent %i \n", pid, parentPid);


    while(1)
    {
    	usleep(1000*2000);
	printf("Client did wait\n");
    }


    printf("Client After sleep\n");

//    while(1) {}
    return 0;
}

