#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include <Sofa.h>
#include <Thread.h>
#include <runtime.h>

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);


    int shellPid = SofaSpawn("shell");
    printf("shell pid is %i\n", shellPid);

    int appStatus = 0;

    while (1)
    {
        pid_t retPid = SofaWait(&appStatus);
        printf("Wait returned pid %i status %i\n", retPid, appStatus);
    }
    
    return 1;
}

