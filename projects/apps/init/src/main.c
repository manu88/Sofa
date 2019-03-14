#include <stdio.h>
#include <unistd.h>
#include <SysClient.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> // mkdir 
#include <errno.h>
#include <assert.h>

int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }


    int ret = seteuid(1);
    printf("seteuid ret %i err %i\n" , ret , errno);

    assert(ret == 0);
    assert(errno == 0);


    uid_t uid = geteuid();
    assert(uid == 1);


    int pidTests = execve("/cpio/TestSysCalls" , NULL , NULL);

    int pidShell = execve("/cpio/shell" , NULL , NULL);

    if (pidShell == -1)
    {
	return 1;
    }
    // wait on any child
    while(1)
    {
	int status = 0;

        pid_t childPid = waitpid(-1, &status , 0);

        printf("pid %i returned \n" , childPid);

    }

    return 0;
}
