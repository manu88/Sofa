#include <SysClient.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

#include <fcntl.h>
#include <string.h>

int main( int argc , char* argv[])
{
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }

    
    errno = 0;
    int consoleFD = open("/dev/console" , O_RDONLY);
    assert(consoleFD >=0);
    assert(errno == 0);

    const char b[] = "Sofa Shell - 2018";
    write(consoleFD , b ,strlen(b));



    int appStatus = 0;
    pid_t childPid = wait(&appStatus);
    assert(childPid == -1);
    assert(errno == ECHILD);

    int retPid = execve("app",NULL , NULL);
    printf("execve returned %i errno %i \n",retPid, errno);
	
    printf("Shell 1 : wait\n");
    childPid = wait(&appStatus);
    assert(childPid >= 1);

    printf("Start child again  /n");

   retPid = execve("app",NULL , NULL);
    printf("execve returned %i errno %i \n",retPid, errno);
        
    printf("Shell 2 : wait\n");
    childPid = wait(&appStatus);
    assert(childPid >= 1);


//    printf("Wait returned %i status %i error %i\n",childPid , appStatus, errno);
    return 0;
}
