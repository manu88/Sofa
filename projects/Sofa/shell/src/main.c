#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include "runtime.h"
#include "files.h"
#include "net.h"
#include <dk.h>

extern seL4_CPtr vfsCap;
extern char* vfsBuf;

int fOut = -1;

void processCommand(const char* cmd);

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
    Printf("Sofa shell\n");
    Printf("Available commands are: exit help ps kill spawn sleep cat poweroff services dk\n");
}

static void doExit(int code)
{
    VFSClientClose(fOut);
    exit(code);
}

static int doSpawn(const char* cmd)
{
    int pid = SFSpawn(cmd);
    if(pid <= 0)
    {
        Printf("Spawn error %i\n", pid);
        return pid;
    }
    int appStatus = 0;
    int ret = SFWait(&appStatus);
    if(ret < 0)
    {
        Printf("Wait interupted\n");
    }
    else
    {
        Printf("%s (pid %i) returned %i\n", cmd, pid, appStatus);
    }

    return 0;
}

static int doLs(const char* path)
{
    DIR *folder = opendir(path);
    if(folder == NULL)
    {
        return errno;
    }
    struct dirent *entry = NULL;
    while( (entry=readdir(folder)) )
    {
        Printf("%s\n", entry->d_name);
    }
    closedir(folder);
}

static int doDK(const char* cmds)
{
    if(strcmp(cmds, "list") == 0)
    {
        int ret = DKClientInit();
        if(ret == 0)
        {
            DKClientEnumDevices();
        }
        else
        {
            Printf("DKClientInit error %i\n", ret);
        }
    }
    else if(strcmp(cmds, "tree") == 0)
    {
        int ret = DKClientInit();
        if(ret == 0)
        {
            DKClientEnumTree();
        }
        else
        {
            Printf("DKClientInit error %i\n", ret);
        }
        
    }
    return 0;
}

static int doCat(const char* path)
{
    if(strlen(path) == 0)
    {
        Printf("cat usage: cat file\n");
        return -EINVAL;
    }
    int handle = open(path, O_RDONLY);
    if(handle <0)
    {
        Printf("open error %i\n", handle);
        return handle;
    }
    uint8_t done = 0;
    while (done == 0)
    {
        char buf[256];
        
        ssize_t ret = read(handle, buf, 256);
        if(ret > 0)
        {
            //buf[ret] = 0;
            for(int i=0;i<ret;i++)
            {
                Printf("%c", isprint(buf[i])?buf[i]:isspace(buf[i])?buf[i]:'#');
            }
            Printf("\n");
        }
        else if(ret == -1) // EOF
        {
            done = 1;
        }
        else
        {
            Printf("Read error %li\n", ret);
            done = 1;
        }

    }
    Printf("\n");
    
    close(handle);
    return 0;
}

static int doSh(const char* cmd)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(cmd, "r");
    if (fp == NULL)
    {
        exit(EXIT_FAILURE);
    }
    while ((read = getline(&line, &len, fp)) != EOF) 
    {
        SFPrintf(":>%s", line);
        processCommand(trim(line));
    }

    fclose(fp);
    return 0;
}

void processCommand(const char* cmd)
{
    if(startsWith("exit", cmd))
    {
        doExit(0);
    }
    else if(startsWith("sh", cmd))
    {
        const char* args = cmd + strlen("sh ");
        doSh(args);
    }
    else if(startsWith("ps", cmd))
    {
        SFDebug(SofaDebugCode_ListProcesses);
    }
    else if(startsWith("poweroff", cmd))
    {
        SFShutdown();
    }
    else if(startsWith("help", cmd))
    {
        cmdHelp();
    }
    else if(startsWith("vfs", cmd))
    {
        VFSClientDebug();
    }
    else if(startsWith("kill", cmd))
    {
        const char *strPid = cmd + strlen("kill ");
        if(strlen(strPid) == 0)
        {
            Printf("Kill usage: kill pid signal\n");
            return;
        }
        pid_t pidToKill = atol(strPid);
        Printf("Kill pid %i\n", pidToKill);
        SFKill(pidToKill, SIGKILL);
    }
    else if(startsWith("dk", cmd))
    {
        const char* cmds = cmd + strlen("dk ");
        doDK(cmds);
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
            Printf("register takes a Name argument\n");
            return;
        }
        Printf("register arg is '%s'\n", serviceName);
        ssize_t ret =  SFRegisterService(serviceName);
        if (ret <= 0)
        {
            Printf("Error registering service '%s' %li\n", serviceName, ret);

        }
        else
        {
            Printf("Service is at %li\n", ret);
        }
        
    }
    else if(startsWith("cat", cmd))
    {
        const char *path = cmd + strlen("cat ");
        doCat(path);
    }
    else if(startsWith("dog", cmd))
    {
        const char *path = cmd + strlen("dog ");
        doCat(path);
    }
    else if(startsWith("ls", cmd))
    {
        const char *path = cmd + strlen("ls ");
        doLs(path);
    }
    else if(startsWith("spawn", cmd))
    {
        const char *strApp = cmd + strlen("spawn ");
        int ret = doSpawn(strApp);
        Printf("%i\n", ret);
    }
    else if(startsWith("wait", cmd))
    {
        int appStatus = 0;
        int ret = SFWait(&appStatus);
        if(ret < 0)
        {
            Printf("Wait interupted\n");
        }
        else
        {
            Printf("wait returned pid %i status %i\n", ret, appStatus);
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
        Printf("PID=%i\n", SFGetPid());
    }
    else if(startsWith("ppid", cmd))
    {
        Printf("PPID=%i\n", SFGetPPid());
    }
    else if(startsWith("dump", cmd))
    {
        SFDebug(SofaDebugCode_DumpSched);
    }
    else
    {
        Printf("Unknown command '%s'\n", cmd);
    }

}

int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);
    argc -=2;
    argv = &argv[2];
    VFSClientInit();

    open("/fake/file1", O_RDONLY); // 0
    open("/fake/cons", O_WRONLY);  // 1
    open("/fake/cons", O_WRONLY);  // 2

    Printf("[%i] Shell has %i args \n", SFGetPid(), argc);

    for(int i=0;i<argc;i++)
    {
        Printf("%i %s\n",i,argv[i]);
    }

    NetClientInit();

    //int h = NetBind(AF_INET, SOCK_DGRAM, 3000);


    while (1)
    {
        Printf(">:");

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
                Printf("[Shell] got ctl-c\n");
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

    doExit(1);
    
    return 1;
}

