#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>


int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] Shell \n", getProcessEnv()->pid);

    while (1)
    {
        char data[128];
        ssize_t readSize = SofaRead(data, 10);
        printf("[shell] %zi '%s'\n", readSize, data);
    }
    
    return 1;
}

