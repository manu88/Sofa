#include "runtime.h"
#include <Sofa.h>


int main(int argc, char *argv[])
{   
    RuntimeInit2(argc, argv);
    SFPrintf("\n\n");
    SFPrintf("[%i] Loader started\n", SFGetPid());

    exit(0);
    return 0;
}

