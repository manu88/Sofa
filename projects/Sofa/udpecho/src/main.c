#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>



int main(int argc, char *argv[])
{   
    
    RuntimeInit2(argc, argv);
    SFPrintf("[%i] started\n", SFGetPid());

    argc -=2;
    argv = &argv[2];

    if(argc < 2)
    {
        SFPrintf("usage: udpecho port\n");
        return 1;
    }

    int inport = atoi(argv[1]);
    SFPrintf("[UDP ECHO] in port %i\n", inport);


    return 0;
}

