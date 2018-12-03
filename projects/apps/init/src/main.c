#include <stdio.h>
#include <unistd.h>
#include <SysClient.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> // mkdir 
#include <errno.h>

int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }

    int pidTests = execve("TestSysCalls" , NULL , NULL);

    int pidShell = execve("shell" , NULL , NULL);

    // wait on any child
    while(1)
    {
	int status = 0;

        pid_t childPid = waitpid(-1, &status , 0);

        printf("pid %i returned \n" , childPid);

    }

    return 0;
}
