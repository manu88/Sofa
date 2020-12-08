#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "runtime.h"


static seL4_CPtr vfsCap = 0;
char* vfsBuf = NULL;

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
            SFPrintf("Error registering service '%s' %li\n", serviceName, ret);

        }
        else
        {
            SFPrintf("Service is at %li\n", ret);
        }
        
    }
    else if(startsWith("close", cmd))
    {
        const char *strArg = cmd + strlen("close ");
        if(strlen(strArg) == 0)
        {
            SFPrintf("close usage: close file handle\n");
            return;
        }
        int handle = atoi(strArg);

        if(vfsCap == 0)
        {
            SFPrintf("[shell] VFS client not registered (no cap)\n");
        }
        if(vfsBuf == NULL)
        {
            SFPrintf("[shell] VFS client not registered(no buff)\n");
        }

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, VFSRequest_Close);
        seL4_SetMR(1, handle);
        seL4_Call(vfsCap, info);
        int err = seL4_GetMR(1);
        SFPrintf("Close returned %i\n", err);
    }
    else if(startsWith("read", cmd))
    {
        const char *strArg = cmd + strlen("read ");
        int handle = -1;
        int size = -1;
        if(sscanf(strArg, "%i %i", &handle, &size) != 2)
        {
            SFPrintf("read usage: read file handle\n");
            return;

        }

        if(vfsCap == 0)
        {
            SFPrintf("[shell] VFS client not registered (no cap)\n");
        }
        if(vfsBuf == NULL)
        {
            SFPrintf("[shell] VFS client not registered(no buff)\n");
        }
        SFPrintf("Read request handle=%i, size=%i\n", handle, size);

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
        seL4_SetMR(0, VFSRequest_Read);
        seL4_SetMR(1, handle);
        seL4_SetMR(2, size);
        seL4_Call(vfsCap, info);
        int err = seL4_GetMR(1);
        int readSize = seL4_GetMR(2);
        SFPrintf("got read response err=%i size=%i\n", err, readSize);


    }
    else if(startsWith("open", cmd))
    {
        const char *path = cmd + strlen("open ");
        if(vfsCap == 0)
        {
            SFPrintf("[shell] VFS client not registered (no cap)\n");
        }
        if(vfsBuf == NULL)
        {
            SFPrintf("[shell] VFS client not registered(no buff)\n");
        }

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
        seL4_SetMR(0, VFSRequest_Open);
        seL4_SetMR(1, 1); // mode
        strcpy(vfsBuf, path);
        vfsBuf[strlen(path)] = 0;
        seL4_Call(vfsCap, info);
        int err = seL4_GetMR(1);
        int handle = seL4_GetMR(2);
        SFPrintf("got Open response err=%i handle=%i\n", err, handle);
    }
    else if(startsWith("ls", cmd))
    {
        const char *path = cmd + strlen("ls ");
        if(vfsCap == 0)
        {
            SFPrintf("[shell] VFS client not registered (no cap)\n");
        }
        if(vfsBuf == NULL)
        {
            SFPrintf("[shell] VFS client not registered(no buff)\n");
        }

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, VFSRequest_ListDir);
        strcpy(vfsBuf, path);
        vfsBuf[strlen(path)] = 0;
        seL4_Call(vfsCap, info);
        SFPrintf("got ls response %lX\n", seL4_GetMR(1));

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

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, VFSRequest_Register);
        seL4_Call(vfsCap, info);
        vfsBuf = (char*) seL4_GetMR(1);
        SFPrintf("VFS client ok\n");

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

