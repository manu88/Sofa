#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "runtime.h"
#include "files.h"
#include "net.h"

extern seL4_CPtr vfsCap;
extern char* vfsBuf;

int fOut = -1;

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
    Printf("Available commands are: exit help ps kill spawn sleep cat poweroff\n");
}

static void doExit(int code)
{
    VFSClose(fOut);
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

void processCommand(const char* cmd)
{
    if(startsWith("exit", cmd))
    {
        doExit(0);
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
        VFSDebug();
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
        if(strlen(path) == 0)
        {
            Printf("cat usage: cat file\n");
            return;
        }

        int handle = VFSOpen(path, O_RDONLY);
        if(handle <0)
        {
            Printf("open error %i\n", handle);
            return;
        }
        uint8_t done = 0;
        while (done == 0)
        {
            char buf[11];
            
            ssize_t ret = VFSRead(handle, buf, 10);
            if(ret > 0)
            {
                buf[ret] = 0;
                Printf("%s", buf);
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
        

        VFSClose(handle);
    }
    else if(startsWith("sendto", cmd))
    {
        const char *strArg = cmd + strlen("sendto ");
        int port = -1;
        char addr[16]  = "";
        char data[128] = "";
        if(sscanf(strArg, "%s %i %s", addr, &port, data) != 3)
        {
            Printf("sendto usage: sendto host port data\n");
            return;
        }
        Printf("send '%s' to '%s':%i\n", data, addr, port);

        struct sockaddr_in server_address;
	    memset(&server_address, 0, sizeof(server_address));
	    server_address.sin_family = AF_INET;
        inet_pton(AF_INET, addr, &server_address.sin_addr);
        server_address.sin_port = htons(port);

        int sock = NetSocket(PF_INET, SOCK_DGRAM, 0);
        NetSendTo(sock, data, strlen(data), 0, (struct sockaddr*)&server_address, sizeof(server_address));
        NetClose(sock);

    }
    else if(startsWith("close", cmd))
    {
        const char *strArg = cmd + strlen("close ");
        if(strlen(strArg) == 0)
        {
            Printf("close usage: close file handle\n");
            return;
        }
        int handle = atoi(strArg);
        int ret = VFSClose(handle);
        Printf("%i\n", ret);
    }
    else if(startsWith("seek", cmd))
    {
        const char *strArg = cmd + strlen("seek ");
        int handle = -1;
        int offset = -1;
        if(sscanf(strArg, "%i %i", &handle, &offset) != 2)
        {
            Printf("seek usage: seek handle offset\n");
            return;
        }
        int ret = VFSSeek(handle, offset);
        Printf("%i\n", ret);
    }
    else if(startsWith("write", cmd))
    {
        const char *strArg = cmd + strlen("write ");
        int handle = -1;
        int size = -1;
        if(sscanf(strArg, "%i %i", &handle, &size) != 2)
        {
            Printf("write usage: write file handle data\n");
            return;
        }
        char data[] = "Hello this is some content";
        int ret = VFSWrite(handle, data, strlen(data));
        Printf("%i\n", ret);
    }
    else if(startsWith("read", cmd))
    {
        const char *strArg = cmd + strlen("read ");
        int handle = -1;
        int size = -1;
        if(sscanf(strArg, "%i %i", &handle, &size) != 2)
        {
            Printf("read usage: read file handle\n");
            return;
        }
        char buf[128];
        ssize_t ret = VFSRead(handle, buf, size);
        if(ret > 0)
        {
            Printf("%li :'%s'\n", ret, buf);
        }
        else if(ret == -1)
        {
            Printf("-- End of file --\n");
        }
        else
        {
            Printf("Read error %li\n", ret);
        }
        
    }
    else if(startsWith("open", cmd))
    {
        const char *args = cmd + strlen("open ");
        int mode = -1;
        char path[512] = "0";
        if(sscanf(args, "%i %s", &mode, path) !=2)
        {
            Printf("usage open Mode path\n");
            return;
        }

        int ret = VFSOpen(path, mode);
        Printf("%i\n",ret);
    }
    else if(startsWith("ls", cmd))
    {
        const char *path = cmd + strlen("ls ");
        if(vfsCap == 0)
        {
            Printf("[shell] VFS client not registered (no cap)\n");
        }
        if(vfsBuf == NULL)
        {
            Printf("[shell] VFS client not registered(no buff)\n");
        }

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, VFSRequest_ListDir);
        strcpy(vfsBuf, path);
        vfsBuf[strlen(path)] = 0;
        seL4_Call(vfsCap, info);
        if(seL4_GetMR(1) != 0)
        {
            Printf("got ls response %lX\n", seL4_GetMR(1));
        }

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
    else if(startsWith("bind", cmd))
    {
        const char *strArgs = cmd + strlen("bind ");

        int port = -1;
        int handle = -1;

        if(sscanf(strArgs, "%i %i", &handle, &port) != 2)
        {
            Printf("bind usage: handle port\n");
            return;
        }
        // socket address used for the server
	    struct sockaddr_in server_address;
	    memset(&server_address, 0, sizeof(server_address));
	    server_address.sin_family = AF_INET;

        server_address.sin_port = htons(port);

	    // htons: host to network long: same as htons but to long
	    server_address.sin_addr.s_addr = htonl(INADDR_ANY);


        int h = NetBind(handle, (struct sockaddr *)&server_address, sizeof(server_address));
        Printf("%i\n", h);

    }
    else if(startsWith("sleep", cmd))
    {
        const char *strMS = cmd + strlen("sleep ");
        int ms = atol(strMS);
        SFSleep(ms);
    }
    else if(startsWith("nclose", cmd))
    {
        const char *str = cmd + strlen("nclose ");
        int handle = atol(str);

        int ret = NetClose(handle);
        Printf("%i\n", ret);
    }
    else if(startsWith("socket", cmd))
    {
        int r = NetSocket(PF_INET, SOCK_DGRAM, 0);
        Printf("socket returned %i\n", r);
    }
    else if(startsWith("r", cmd))
    {
        const char *strArgs = cmd + strlen("r ");

        int size = -1;
        int handle = -1;

        if(sscanf(strArgs, "%i %i", &handle, &size) != 2)
        {
            Printf("bind usage: handle port\n");
            return;
        }

        char dats[128];
        if(size > 128)
        {
            size = 128;
        }

        struct sockaddr_in client_address;
	    int client_address_len = 0;

        ssize_t rRead = NetRecvFrom(0, dats, size, 0, (struct sockaddr *)&client_address, &client_address_len);
        Printf("NetRecvFrom returned %zi '%s'\n", rRead, dats);
        if(rRead)
        {
            Printf("Received %zi msg from %s on port %i\n", rRead, inet_ntoa(client_address.sin_addr), client_address.sin_port );
        }

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
    else if(startsWith("dev", cmd))
    {
        SFDebug(SofaDebugCode_ListDevices);
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

    fOut = VFSOpen("/fake/cons", O_WRONLY);

    Printf("[%i] Shell has %i args \n", SFGetPid(), argc);

    for(int i=0;i<argc;i++)
    {
        Printf("%i %s\n",i,argv[i]);
    }

    NetInit();

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

