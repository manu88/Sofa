#include "runtime.h"
#include <Sofa.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dirent.h>
#include <proc.h>
#include <signal.h>
#include <Thread.h>

static void testMmap()
{
    char* ptr = mmap(NULL, 4096, 0, 0,-1, 0);
    assert(ptr);
    ptr[0] = 1;
    ptr[4095] = 2;
    int ret = munmap(ptr, 4096);
    assert(ret == 0);
}

static void testMalloc(void)
{
    void *ptr = malloc(512);
    assert(ptr);
    free(ptr);
}
static void testRead(void)
{
    int h = open("/fake/file1", O_RDONLY);
    assert(h != -1);

    char b[10] = "";
    ssize_t ret = read(h, b, 10);
    while (ret != EOF)
    {
        ret = read(h, b, 10);
        if(ret > 0)
            b[ret] = 0;
    }
    close(h);
}

static void testWait()
{
    assert(ProcClientWait(NULL) == -ECHILD);
}

static void testReaddir()
{
    DIR *folder = opendir("/");
    if(folder == NULL)
    {
        return;
    }
    struct dirent *entry = NULL;
    while( (entry=readdir(folder)) )
    {
    }
    closedir(folder);

}

static void testSpawn(const char* selfName)
{
    char b[512];
    snprintf(b,512, "%s 1", selfName);
    int pid = ProcClientSpawn(b);
    assert(pid > 0);
    int status = 0;
    int r = ProcClientWaitPid(pid, &status, 0);
    assert(r == pid);
    assert(WIFEXITED(status));
}

static void testKillChild(const char* selfName)
{
    char b[512];
    snprintf(b,512, "%s 2", selfName);
    int pid = ProcClientSpawn(b);
    assert(pid > 0);

    int r = ProcClientKill(pid, SIGKILL);
    assert(r == 0);
}

static void testChildFault(const char* selfName)
{
    char b[512];
    snprintf(b,512, "%s 3", selfName);
    int pid = ProcClientSpawn(b);
    assert(pid > 0);

    int status = 0;
    int r = ProcClientWaitPid(pid, &status, 0);
    assert(r == pid);
    assert(WIFSIGNALED(status));
}

static Thread th;

int thread1(Thread* thread, void *arg)
{
    assert(thread == &th);
    int val = (int) arg;
    assert(val == 42);

    SFSleep(1000);
    return 53;
}

static void testThread()
{
    //ThreadInit(&th, thread1, (void*) 42);

}


static int baseMain(int argc, char *argv[])
{
    testRead();
    testMmap();
    testReaddir();
    testMalloc();
    testWait();
    testSpawn(argv[0]);
    testKillChild(argv[0]);
    testChildFault(argv[0]);

    testThread();

    return 0;
}

static int mode1()
{
    return 1;
}

static int mode2()
{
    while (1)
    {
    }
    return 1;
}

static int mode3()
{
    float* lolz = NULL;
    *lolz = 42;
}

int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);

    argc -=2;
    argv = &argv[2];

    VFSClientInit();
    ProcClientInit();

//    SFPrintf("Unit tests '%s' %i args\n", argv[0], argc);

    if(argc == 1)
    {
        return baseMain(argc, argv);
    }
    else
    {
        int mode = atoi(argv[1]);
        switch (mode)
        {
        case 1:
            return mode1();
            break;
        case 2:
            return mode2();
            break;
        case 3:
            return mode3();
            break;
        
        default:
            break;
        }
    }
    
    return 0;
}

