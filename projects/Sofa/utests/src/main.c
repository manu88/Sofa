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

#define DO_LOG 1

#ifdef DO_LOG
#define TEST_LOG(args...) SFPrintf(args)
#else
#define TEST_LOG(args...)
#endif

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

//static int h = -1;
static void testRead(void)
{
    char b[11] = "";
    int h = open("/fake/file1", O_RDONLY);
    int guard = 10;
    assert(h != -1);
    assert(guard==10);

    ssize_t ret = read(h, b, 10);
    assert(guard==10);

    while (ret != EOF)
    {

        ret = read(h, b, 10);
        assert(guard==10);

        if(ret > 0)
        {
            b[ret] = 0;
        }
    }


    close(h);
}

static void testTime()
{
    uint64_t t1 = SFGetTime();
    assert(SFSleep(100) == 0);
    uint64_t t2 = SFGetTime();
    assert(t2 > t1);
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

    SFSleep(500);
    return 53;
}

static void testThread()
{
    ThreadInit(&th, thread1, (void*) 42);
    int retVal = 0;
    assert(ThreadJoin(&th, &retVal) == 0);
    assert(retVal == 53);

}


static int baseMain(int argc, char *argv[])
{
    TEST_LOG("[Unit tests] test read\n");
    testRead();

    TEST_LOG("[Unit tests] test mmap\n");
    testMmap();

    TEST_LOG("[Unit tests] test read dir \n");
    testReaddir();

    TEST_LOG("[Unit tests] test malloc\n");
    testMalloc();

    TEST_LOG("[Unit tests] test wait\n");
    testWait();

    TEST_LOG("[Unit tests] test spawn\n");
    testSpawn(argv[0]);

    TEST_LOG("[Unit tests] test kill child\n");
    testKillChild(argv[0]);

    TEST_LOG("[Unit tests] test child fault\n");
    testChildFault(argv[0]);

    TEST_LOG("[Unit tests] test time\n");
    testTime();

    //testThread();

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


    if(argc == 1)
    {
        TEST_LOG("[Unit tests] base main\n");
        return baseMain(argc, argv);
    }
    else
    {
        int mode = atoi(argv[1]);
        switch (mode)
        {
        case 1:
            TEST_LOG("[Unit tests] mode1\n");
            return mode1();
            break;
        case 2:
            TEST_LOG("[Unit tests] mode2\n");
            return mode2();
            break;
        case 3:
            TEST_LOG("[Unit tests] mode3\n");
            return mode3();
            break;
        
        default:
            TEST_LOG("[Unit tests] unknown mode (%i)\n", mode);
            break;
        }
    }
    
    return 0;
}

