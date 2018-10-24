#include <SysClient.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

int main( int argc , char* argv[])
{
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }


    int appStatus = 0;

    pid_t childPid = wait(&appStatus);
    assert(childPid == -1);
    assert(errno == ECHILD);

    int retPid = execve("app",NULL , NULL);
    printf("execve returned %i errno %i \n",retPid, errno);

//    printf("Wait returned %i status %i error %i\n",childPid , appStatus, errno);
    return 0;
}
