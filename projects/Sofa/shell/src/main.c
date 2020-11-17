#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "runtime.h"


static char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;
    
    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }
    
    len = strlen(str);
    endp = str + len;
    
    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while(isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while(isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }
    
    if( str + len - 1 != endp )
        *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
        *str = '\0';
    
    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
        while( *frontp ) { *endp++ = *frontp++; }
        *endp = '\0';
    }
    
    
    return str;
}

static int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
    lenstr = strlen(str);
    
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void cmdHelp()
{
    printf("Sofa shell\n");
    printf("Available commands are: exit help ps kill spawn\n");
}

void processCommand(const char* cmd)
{
    if(startsWith("exit", cmd))
    {
        exit(0);
    }
    else if(startsWith("ps", cmd))
    {
        SofaDebug(SofaDebugCode_ListProcesses);
    }
    else if(startsWith("help", cmd))
    {
        cmdHelp();
    }
    else if(startsWith("kill", cmd))
    {
        const char *strPid = cmd + strlen("kill ");
        if(strlen(strPid) == 0)
        {
            
            printf("Kill usage: kill pid signal\n");
            return;
        }
        pid_t pidToKill = atol(strPid);
        printf("Kill pid %i\n", pidToKill);
        SofaKill(pidToKill, SIGKILL);
    } 
    else if(startsWith("spawn", cmd))
    {
        const char *strApp = cmd + strlen("spawn ");
        int pid = SofaSpawn(strApp);
        int appStatus = 0;
        pid_t ret = SofaWait(&appStatus);
        printf("%s (pid %i) returned %i\n", strApp, pid, appStatus);
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
            processCommand(trim(data));
        }

    }
    
    return 1;
}

