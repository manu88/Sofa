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
        int c = SofaReadChar();
        if(c > 0)
        {
            printf("%c\n", c);
        }
    }
    
    return 1;
}

