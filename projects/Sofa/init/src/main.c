#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include <Sofa.h>
#include <Thread.h>
#include <runtime.h>

int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);

    if(SFGetPid() != 1)
    {
        return EXIT_FAILURE;
    }
    const char shellPath[] = "/cpio/shell";
    int shellPid = SFSpawn(shellPath);
    SFPrintf("[init] shell pid is %i\n", shellPid);

    int appStatus = 0;

    while (1)
    {
        pid_t retPid = SFWait(&appStatus);
        SFPrintf("[init] Wait returned pid %i status %i\n", retPid, appStatus);
        if(retPid == shellPid)
        {
            shellPid = SFSpawn(shellPath);
            SFPrintf("[init] shell pid is %i\n", shellPid);
        }
    }
    return 1;
}

