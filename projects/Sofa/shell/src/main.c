#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include "runtime.h"

static int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
    lenstr = strlen(str);
    
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void processCommand(const char* cmd)
{
    if(startsWith("exit", cmd))
    {
        exit(0);
    }
    else if (startsWith("ps", cmd))
    {
        SofaDebug(SofaDebugCode_ListProcesses);
    }
    else
    {
        printf("Unknown command '%s'\n", cmd);
    }

}

int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);
    printf("\n\n");
    fflush(stdout);
    printf("[%i] Shell \n", SofaGetPid());

    while (1)
    {
        printf(">:");
        fflush(stdout);

        char data[128] = "";
        uint8_t gotCmd = 0;
        size_t bufferIndex = 0;
        ssize_t readSize = 0;
        while (gotCmd == 0)
        {
            const size_t sizeToRead = 16;
            readSize = SofaReadLine(data + bufferIndex, sizeToRead);
            if(readSize == -EAGAIN)
            {
                bufferIndex += sizeToRead;
            }
            else
            {
                gotCmd = 1;
                data[bufferIndex + readSize -1] = 0;
            }
        }
        if(strlen(data))
        {
            processCommand(data);
        }

    }
    
    return 1;
}

