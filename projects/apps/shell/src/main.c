#include <SysClient.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <errno.h>


int main( int argc , char* argv[])
{
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }


    int retPid = execve("app",NULL , NULL);
    
    printf("execve returned %i errno %i \n",retPid, errno);

    int appStatus = 0;

    
    pid_t childPid = wait(&appStatus);
    printf("Wait returned %i status %i error %i\n",childPid , appStatus, errno);
    return 0;
}
