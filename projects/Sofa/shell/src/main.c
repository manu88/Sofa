#include <Sofa.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <runtime.h>
#include <files.h>
#include <proc.h>
#include <dk.h>
#include <init.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <net.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int fOut = -1;
static int lastCmdRet = 0;
int processCommand(const char* cmd);

static char* currentWD = NULL;

static void showPrompt()
{
    if(currentWD == NULL)
    {
        currentWD = get_current_dir_name();
    }
    Printf("%s$ ", currentWD);
}

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
    Printf("Available commands are: echo exit help ps sh kill spawn sleep cat poweroff services dk gettime cd pwd\n");
}

static void doExit(int code)
{
    VFSClientClose(fOut);
    exit(code);
}

static int doSpawn(char* cmd)
{
    int detached = cmd[strlen(cmd)-1] == '&'? 1:0;
    if(detached)
    {
        cmd[strlen(cmd)-1] = 0;
    }
    int pid = ProcClientSpawn(cmd);
    if(pid <= 0)
    {
        Printf("Spawn error %i\n", pid);
        return pid;
    }
    if(!detached)
    {
        int appStatus = 0;
        int ret = waitpid(pid, &appStatus, 0);
        if(ret < 0)
        {
            Printf("Wait interupted\n");
        }
        else
        {
            Printf("%s (pid %i) returned %i\n", cmd, pid, appStatus);
        }
    }
    else
    {
        Printf("[ ] %i\n", pid);
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
    struct stat buffer;

    char pathB[256] = "";
    while( (entry=readdir(folder)))
    {
        pathB[0] = 0;
        if(strlen(path))
        {
            strcat(pathB, path);
            size_t pathLen = strlen(pathB);
            if(pathB[pathLen-1] != '/')
            {
                strcat(pathB, "/");
            }
        }

        strcat(pathB, entry->d_name);
        int status = stat(pathB, &buffer);
        Printf("%s ", entry->d_name);
        if(status == 0)
        {
            switch (buffer.st_mode & S_IFMT)
            {
                case S_IFBLK:  Printf("block device");            
                    break;
                case S_IFCHR:  Printf("character device");        
                    break;
                case S_IFDIR:  Printf("directory");               
                    break;
                case S_IFIFO:  Printf("FIFO/pipe");               
                    break;
                case S_IFLNK:  Printf("symlink");                 
                    break;
                case S_IFREG:  Printf("regular file");            
                    break;
                case S_IFSOCK: Printf("socket");                  
                    break;
                default:       Printf("unknown(%X)", buffer.st_mode & S_IFMT);                
                    break;
           }
        }
        else
        {
            Printf(" stat error=%i", status);
        }
        Printf("\n");
    }
    closedir(folder);

    return 0;
}

static int doDK(const char* cmds)
{
    if(startsWith("enum", cmds))
    {
        const char* strType = cmds + strlen("enum ");
        if(strlen(strType)==0)
        {
            return -EINVAL;
        }
        int type = atoi(strType);
        if(type > DKDevice_AllTypes)
        {
            return -1;
        }

        size_t numDev = 0;
        int ret = DKClientEnumDevices(type, NULL, &numDev);
        if(ret == 0)
        {
            Printf("Found %zi devices matching type %i\n", numDev, type);
            if(numDev)
            {
                DKDeviceList *list = malloc(sizeof(DKDeviceList) + (sizeof(DKDeviceHandle)*numDev));
                assert(list);
                size_t numDev2 = 0;
                ret = DKClientEnumDevices(type, list, &numDev2);
                assert(numDev2 == numDev);
                assert(numDev == list->count);

                for(size_t i=0; i<list->count; i++)
                {
                    char* name = DKDeviceGetName(list->handles[i]);
                    char* devFile = DKDeviceGetDevFile(list->handles[i]);
                    Printf("DKDevice at %lu %s %s\n", list->handles[i], name, devFile);
                    free(name);
                    free(devFile);
                }

                free(list);
            }
        }
        return ret;
    }
    else if(startsWith("write", cmds))
    {
        const char* params = cmds + strlen("write ");

        DKDeviceHandle handle = atoll(params);

        const char data[] = "Hello device";
        return DKDeviceWrite(handle, 0, data, strlen(data));
    }
    else if(strcmp(cmds, "tree") == 0)
    {
        return DKClientEnumTree();
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
        }
        else if(ret == -1) // EOF
        {
            done = 1;
        }
        else
        {
            done = 1;
        }

    }
    
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
        return -ENOENT;
    }
    while ((read = getline(&line, &len, fp)) != EOF) 
    {
        processCommand(trim(line));
    }

    fclose(fp);
    return 0;
}

static int PSOnProcessDescription(const ProcessDesc* desc, void* _)
{
    uint64_t currentT = SFGetTime();
    Printf("PID %i '%s' %u pages:%zi start time %li running %f \n",
           desc->pid,
           desc->name,
           desc->state,
           desc->numPages,
           desc->startTime,
           (float)(currentT- desc->startTime) / NS_IN_S);

    return 0;
}

static int doPS(const char* cmd)
{
    return ProcClientEnum(PSOnProcessDescription, NULL);
}

static int doKill(const char* args)
{
    const char *strPid = args;
    if(strlen(strPid) == 0)
    {
        Printf("Kill usage: kill pid signal\n");
        return -1;
    }
    pid_t pidToKill = atol(strPid);
    Printf("Kill pid %i\n", pidToKill);
    ProcClientKill(pidToKill, SIGKILL);
}

static int doInit(const char* args)
{
    ssize_t cap = SFGetService("init");
    Printf("init connection %zi\n", cap);
    if(cap > 0)
    {
        seL4_MessageInfo_t msg = seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0, 1);
        seL4_SetMR(0, InitRequest_Connect);
        msg = seL4_Call(cap, msg);
        long addr = seL4_GetMR(0);
        char* sharedBuf = (char*) addr;
        SFPrintf("init responded to code 1\n");

        msg = seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0, 1);
        seL4_SetMR(0, InitRequest_Test);
        sprintf(sharedBuf, "Hello world");
        seL4_Send(cap, msg);

        return 0;
    }

    return cap;
}


static int doEcho(const char* args)
{
    const char* trimmed = trim(args);
    if(strlen(trimmed) == 0)
    {
        return -EINVAL;
    }
    if(strcmp(trimmed, "$?") == 0)
    {
        Printf("%i\n", lastCmdRet);
    }
    else if(strcmp(trimmed, "$$") == 0)
    {
        Printf("%i\n", getpid());
        return 0;
    }
    else if(trimmed[0] == '$')
    {
        const char* name = trimmed + 1;
        char* value = getenv(name);
        Printf("%s\n", value);
    }
    else
    {
        Printf("'%s'\n", trim(args));
    }
    
    return 0;
}




static void putPixel(void* fb, int x, int y, uint32_t color)
{

    unsigned int coord_factor = 4;
    size_t len = 3;
    char* target = ((char *)fb) + (y * 640 + x) * coord_factor;

    target[0] = color & 255;
    target[1] = (color >> 8) & 255;
    target[2] = (color >> 16) & 255;
}

int processCommand(const char* cmd)
{
    if(startsWith("exit", cmd))
    {
        doExit(0);
        return 0; 
    }
    else if(startsWith("echo", cmd))
    {
        const char* args = cmd + strlen("echo ");
        return doEcho(args);
    }
    else if(startsWith("sh", cmd))
    {
        const char* args = cmd + strlen("sh ");
        doSh(args);
    }
    else if(startsWith("ps", cmd))
    {
        const char* args = cmd + strlen("ps ");
        return doPS(args);
    }
    else if(startsWith("poweroff", cmd))
    {
        return SFShutdown();
    }
    else if(startsWith("help", cmd))
    {
        cmdHelp();
        return 0;
    }
    else if(startsWith("kill", cmd))
    {
        const char *args = cmd + strlen("kill ");
        return doKill(args);
    }
    else if(startsWith("dk", cmd))
    {
        const char* cmds = cmd + strlen("dk ");
        return doDK(cmds);
    }
    else if(startsWith("services", cmd))
    {
        SFDebug(SofaDebugCode_ListServices);
        return 0;
    }
    else if(startsWith("malloc", cmd))
    {
        size_t size = atol(cmd + strlen("malloc "));
        Printf("malloc'ing %zi bytes\n", size);
        void* ptr = malloc(size);
        Printf("%p\n", ptr);
    }
    else if(startsWith("init", cmd))
    {
        const char *args = cmd + strlen("init ");
        return doInit(args);
    }
    else if(startsWith("cat", cmd))
    {
        const char *path = cmd + strlen("cat ");
        return doCat(path);
    }
    else if(startsWith("dog", cmd))
    {
        const char *path = cmd + strlen("dog ");
        return doCat(path);
    }
    else if(startsWith("ls", cmd))
    {
        const char *path = cmd + strlen("ls ");
        return doLs(path);
    }
    else if(startsWith("write ", cmd))
    {
        const char *args = cmd + strlen("write");

        char path[128] = "";
        char *data = NULL;
        int n = 0;
        sscanf(args, "%s %n", path, & n);

        data = args + n;


        FILE* f = fopen(path,"w");
        if(f == NULL)
            return errno;
        fprintf(f, "%s", data);
        fclose(f);
        return 0;
    }
    else if(startsWith("spawn", cmd))
    {
        char *strApp = (char*)(cmd + strlen("spawn "));
        int ret = doSpawn(strApp);
        return ret;
    }
    else if(startsWith("wait", cmd))
    {
        int appStatus = 0;
        int ret = wait(&appStatus);
        if(ret < 0)
        {
            Printf("Wait interupted\n");
        }
        return ret;
    }
    else if(startsWith("sleep", cmd))
    {
        const char *strMS = cmd + strlen("sleep ");
        int ms = atol(strMS);
        return SFSleep(ms);
    }
    else if(strcmp("gettime", cmd) == 0)
    {
        float tsecs = (float)SFGetTime()/NS_IN_S;
        Printf("Time %f\n", tsecs);
        return 0;
    }
    else if(startsWith("pid", cmd))
    {
        pid_t pid = getpid();
        Printf("PID=%i\n", pid);
        return pid;
    }
    else if(startsWith("ppid", cmd))
    {
        pid_t ppid = getppid();
        Printf("PPID=%i\n", ppid);
        return ppid;
    }
    else if(startsWith("pwd", cmd))
    {
        Printf("%s\n", currentWD);
        return 0;

    }
    else if(startsWith("cd ", cmd))
    {
        const char *p = cmd + strlen("cd ");
        int ret = chdir(p);
        if(ret == 0)
        {
            free(currentWD);
            currentWD = get_current_dir_name();
        }
        return ret;
    }
        else if(startsWith("traces", cmd))
    {
        SFDebug(SofaDebugCode_EnableSyscallTraces);
        return 0;
    }
    else if(startsWith("dump", cmd))
    {
        SFDebug(SofaDebugCode_DumpSched);
        return 0;
    }
    else if(startsWith("ip", cmd))
    {
        return NetClientEnumInterfaces();
    }
    else if(startsWith("sendto", cmd))
    {
        int sock;
        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) 
        {
            Printf("could not create socket\n");
            return errno;
        }
        struct sockaddr_in si_other;
        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(3000);
        if (inet_aton("127.0.0.1" , &si_other.sin_addr) == 0) 
        {
            fprintf(stderr, "inet_aton() failed\n");
            return errno;
        }

        const char buffer[] = "hello world";
        ssize_t retSend = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&si_other,
		       sizeof(si_other));
        if(retSend == -1)
        {
            return errno;
        }
        return retSend;

    }
    else if(startsWith("vga", cmd))
    {
        static void* fb = NULL;

        if(fb == NULL)
        {
            DKDeviceHandle devHandle = DKClientGetDeviceNamed("Framebuffer", DKDevice_FrameBuffer);
            if(devHandle == DKDeviceHandle_Invalid)
            {
                Printf("vga: invalid dev handle");
                return -1;
            }

            long ret = DKDeviceMMap(devHandle, 1);
            Printf("vga: ret = %li\n", ret);
            if(ret > 0)
            {
                fb = (void*) ret;
            }
        }
        if(fb)
        {
            for(int i=0;i<200;i++)
            {
                putPixel(fb, i, i, 0X00FF0000);
            }
        }
    }
    else if(startsWith("net", cmd))
    {
        const char* args = cmd + strlen("net ");
        char devName[256] = "";
        if(sscanf(args, "%s", devName) != 1)
        {
            return -1;
        }

        DKDeviceHandle devHandle = DKClientGetDeviceNamed(devName, DKDevice_Net);

        if(devHandle == DKDeviceHandle_Invalid)
        {
            Printf("net: invalid dev handle");
            return -1;
        }
        Printf("Got dev handle %lu for %s\n", devHandle, devName);
        return NetClientConfigureInterface(devHandle, "10.0.2.15", "192.168.0.1", "255.255.255.*");
    }
    else
    {
        Printf("Unknown command '%s'\n", cmd);
    }
    return -1;
}

int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);
    argc -=2;
    argv = &argv[2];
    VFSClientInit();
    DKClientInit();

    Printf("[%i] Shell has %i args \n", getpid(), argc);

    if(argc > 1)
    {
        close(2);
        close(1);
        close(0);

        open("/dev/tty1", O_RDONLY);  // 0
        open("/dev/tty1", O_WRONLY);  // 1
        open("/dev/tty1", O_WRONLY);  // 2        
    }

    ProcClientInit();

    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    FILE* fp = stdin;
    showPrompt();

    while ((read = getline(&line, &len, fp)) != EOF) 
    {
        line[read-1] = 0;

        if(strlen(line))
        {
            lastCmdRet = processCommand(trim(line));
        }
        showPrompt();
    }

    doExit(1);
    
    return 1;
}

