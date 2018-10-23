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


    usleep(1000*1000);

//    while(1) {}
    return 0;
}

