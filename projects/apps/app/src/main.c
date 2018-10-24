#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SysClient.h>

#include <errno.h>
#include <signal.h>
#include <assert.h>
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

    int retNice = nice(-20);
    printf("Client nice returned %i errno %i\n",retNice, errno);

    errno = 0;
//    kill(1 , SIGCONT);
    while(1)
    {
    	int ret = usleep(1000*4000);
	assert(ret == 0);
	assert(errno == 0);
        printf("Client %i did wait\n" , pid);
    }


    printf("Client After sleep\n");

    return 0;
}

