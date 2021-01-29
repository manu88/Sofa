#include "runtime.h"
#include <Sofa.h>
#include <files.h>
#include "lib-support.h"

int main(int argc, char *argv[])
{   
    RuntimeInit2(argc, argv);
    VFSClientInit();
    SFPrintf("\n\n");
    SFPrintf("[%i] Loader started\n", SFGetPid());


    dloader_p o = DLoader.load("/ext/test_lib.so");

    exit(0);
    return 0;
}

