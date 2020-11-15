#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include <Sofa.h>
#include <Thread.h>
#include <runtime.h>

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    
    printf("[INIT] started\n");

    int appPid = SofaSpawn("app");

    printf("app pid is %i\n", appPid);

    int appStatus = 0;
    pid_t retPid = SofaWait(&appStatus);
    printf("Wait returned %i status %i\n", retPid, appStatus);    

    return 1;
}

