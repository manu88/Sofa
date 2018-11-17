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

    errno = 0;
    int retMkDir = mkdir("folder" , 0666);
    printf("retMkDir %i errno %i \n" , retMkDir , errno);


    int pidTests = execve("TestSysCalls" , NULL , NULL);


    int pidShell = execve("shell" , NULL , NULL);

    int status = 0;
    waitpid(pidShell, &status , 0);

    return 0;
}
