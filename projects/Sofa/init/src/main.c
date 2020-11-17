#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include <Sofa.h>
#include <Thread.h>
#include <runtime.h>

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);


    int shellPid = SofaSpawn("shell");
    printf("[init] shell pid is %i\n", shellPid);

    int appStatus = 0;

    while (1)
    {
        pid_t retPid = SofaWait(&appStatus);
        printf("[init] Wait returned pid %i status %i\n", retPid, appStatus);
        if(retPid == shellPid)
        {
            shellPid = SofaSpawn("shell");
            printf("[init] shell pid is %i\n", shellPid);
        }
    }
    return 1;
}

