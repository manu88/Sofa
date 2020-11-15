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

    SofaSleep(10000);
    while (1)
    {
        seL4_Yield();
    }
    

    return 1;
}

