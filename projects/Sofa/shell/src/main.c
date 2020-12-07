#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "runtime.h"


static seL4_CPtr vfsCap = 0;

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
    SFPrintf("Sofa shell\n");
    SFPrintf("Available commands are: exit help ps kill spawn sleep\n");
}

void processCommand(const char* cmd)
{
    if(startsWith("exit", cmd))
    {
        exit(0);
    }
    else if(startsWith("ps", cmd))
    {
        SFDebug(SofaDebugCode_ListProcesses);
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
            SFPrintf("Kill usage: kill pid signal\n");
            return;
        }
        pid_t pidToKill = atol(strPid);
        SFPrintf("Kill pid %i\n", pidToKill);
        SFKill(pidToKill, SIGKILL);
    }
    else if(startsWith("services", cmd))
    {
        SFDebug(SofaDebugCode_ListServices);
    }
    else if(startsWith("register", cmd))
    {
        const char* serviceName = cmd + strlen("register ");
        if(strlen(serviceName) == 0)
        {
            SFPrintf("register takes a Name argument\n");
            return;
        }
        SFPrintf("register arg is '%s'\n", serviceName);
        ssize_t ret =  SFRegisterService(serviceName);
        if (ret <= 0)
        {
            SFPrintf("Error registering service '%s' %i\n", serviceName, ret);

        }
        else
        {
            SFPrintf("Service is at %i\n", ret);
        }
        
    }
    else if(startsWith("ls", cmd))
    {
        //const char *path = cmd + strlen("ls ");
        //SFVFS(VFSRequest_ListDir, path, strlen(path));

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
        seL4_SetMR(0, 33);
        seL4_Send(vfsCap, info);

    }
    else if(startsWith("spawn", cmd))
    {
        const char *strApp = cmd + strlen("spawn ");
        int pid = SFSpawn(strApp);
        if(pid <= 0)
        {
            SFPrintf("Spawn error %i\n", pid);
            return;
        }
        int appStatus = 0;
        int ret = SFWait(&appStatus);
        if(ret < 0)
        {
            SFPrintf("Wait interupted\n");
        }
        else
        {
            SFPrintf("%s (pid %i) returned %i\n", strApp, pid, appStatus);
        }
    }
    else if(startsWith("wait", cmd))
    {
        int appStatus = 0;
        int ret = SFWait(&appStatus);
        if(ret < 0)
        {
            SFPrintf("Wait interupted\n");
        }
        else
        {
            SFPrintf("wait returned pid %i status %i\n", ret, appStatus);
        }
    }
    else if(startsWith("sleep", cmd))
    {
        const char *strMS = cmd + strlen("sleep ");
        int ms = atol(strMS);
        SFSleep(ms);
    }
    else if(startsWith("pid", cmd))
    {
        SFPrintf("PID=%i\n", SFGetPid());
    }
    else if(startsWith("ppid", cmd))
    {
        SFPrintf("PPID=%i\n", SFGetPPid());
    }
    else if(startsWith("dump", cmd))
    {
        SFDebug(SofaDebugCode_DumpSched);
    }
    else if(startsWith("dev", cmd))
    {
        SFDebug(SofaDebugCode_ListDevices);
    }
    else
    {
        SFPrintf("Unknown command '%s'\n", cmd);
    }

}

int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);
    SFPrintf("[%i] Shell \n", SFGetPid());

    ssize_t capOrErr = SFGetService("VFS");
    SFPrintf("SFGetService returned %li\n", capOrErr);

    if(capOrErr > 0)
    {
        vfsCap = capOrErr;
    }
    while (1)
    {
        SFPrintf(">:");
        fflush(stdout);

        char data[128] = "";
        uint8_t gotCmd = 0;
        size_t bufferIndex = 0;
        ssize_t readSize = 0;
        while (gotCmd == 0)
        {
            const size_t sizeToRead = 16;
            readSize = SFReadLine(data + bufferIndex, sizeToRead);
            if(readSize == -EINTR)
            {
                SFPrintf("[Shell] got ctl-c\n");
            }
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

