#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SysClient.h>


#include <signal.h>

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



//    kill(1 , SIGCONT);
    while(1)
    {
    	usleep(1000*4000);
        printf("Client %i did wait\n" , pid);
    }


    printf("Client After sleep\n");

    return 0;
}

