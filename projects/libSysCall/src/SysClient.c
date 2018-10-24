#include "SysClient.h"

//#include <sys/wait.h>
#include <unistd.h> // pid_t
#include <stdio.h>
#include <sel4/sel4.h>
#include <stdlib.h> // atol

#include <utils/time.h>
#include <time.h>

#include <muslcsys/vsyscall.h>
#include <assert.h>
#include <errno.h>
#include <string.h> 


#include <SysCallNum.h>


static long sys_nanosleep(va_list args);
static long sys_getpid(va_list args);
static long sys_getppid(va_list args);
static long sys_exit(va_list args);
static long sys_kill(va_list args);
static long sys_wait4(va_list args);
static long sys_execve(va_list args);

static seL4_CPtr sysCallEndPoint = 0;

int SysClientInit(int argc , char* argv[] )
{
    sysCallEndPoint = (seL4_CPtr) atol(argv[0]);

    if (!sysCallEndPoint)
    {
        return 1;
    }
    
    muslcsys_install_syscall(__NR_nanosleep , sys_nanosleep);
    muslcsys_install_syscall(__NR_getpid ,    sys_getpid);
    muslcsys_install_syscall(__NR_getppid ,   sys_getppid);
    muslcsys_install_syscall(__NR_exit,       sys_exit);
    muslcsys_install_syscall(__NR_kill,       sys_kill);
    muslcsys_install_syscall(__NR_execve,     sys_execve);
    muslcsys_install_syscall(__NR_wait4,     sys_wait4);

    return 0;
}

//pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);

static long sys_wait4(va_list args)
{
    const pid_t pid = va_arg(args , pid_t);
    int *wstatus    = va_arg(args , int*);
    const int options     = va_arg(args , int);
    struct rusage *rusage = va_arg(args ,struct rusage *);

    printf("sys_wait4 : pid %i options %i\n",pid,options);
    
    seL4_MessageInfo_t tag;
    seL4_Word msg;
    
    tag = seL4_MessageInfo_new(0, 0, 0, 3 );
    seL4_SetMR(0, __SOFA_NR_wait4);
    seL4_SetMR(1, pid);
    seL4_SetMR(2, options);

    tag = seL4_Call(sysCallEndPoint, tag);


    msg = seL4_GetMR(1); // ret code}

    return msg;
}
//int execve(const char *filename, char *const argv[], char *const envp[]);
static long sys_execve(va_list args)
{
    const char* filename = va_arg(args , char*);

    if (!filename)
    {
        return -EFAULT;
    }

//    printf("Execve : '%s'\n", filename);
    seL4_MessageInfo_t tag;
    seL4_Word msg;
    
    tag = seL4_MessageInfo_new(0, 0, 0, 1/*syscall num*/ + strlen(filename) );
    seL4_SetMR(0, __SOFA_NR_execve);

    for( int i=0;i<strlen(filename);++i)
    {
        seL4_SetMR(1 +i , filename[i]);
    }
    tag = seL4_Call(sysCallEndPoint, tag);


    msg = seL4_GetMR(1); // ret code
    
    return (int)msg;

//    return ENOSYS;
}

//int kill(pid_t pid, int sig);
static long sys_kill(va_list args)
{
    const pid_t pid  = va_arg (args, pid_t);
    const int sig = va_arg (args, int);
    
    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 3);

    seL4_SetMR(0,  __SOFA_NR_kill);
    seL4_SetMR(1 , pid);
    seL4_SetMR(2 , sig);

    tag = seL4_Call(sysCallEndPoint, tag);


    printf("Got kill response\n");
/*
    if(seL4_MessageInfo_get_length(tag) != 2)
    {
        return -1; // no posix compliant ; getpid should no return any error
    }
*/
    msg = seL4_GetMR(1); // ret code
 

    return (int)msg;
}


static long sys_nanosleep(va_list args)
{
    struct timespec *req = va_arg(args, struct timespec*);
    /* construct a sleep call */
    int millis = req->tv_sec * MS_IN_S;
    millis += req->tv_nsec / NS_IN_MS;
//    printf("Usleep %i ms\n",millis);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, __SOFA_NR_nanosleep);
    seL4_SetMR(1 , millis);

    tag = seL4_Call(sysCallEndPoint, tag);

    assert(seL4_MessageInfo_get_length(tag) >=1 );

    seL4_Word sysCallNum = seL4_GetMR(0);

    assert(sysCallNum == __NR_nanosleep);

    return 0;
}

static void Abort()
{
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 1);
    seL4_SetMR(0, __SOFA_NR_exit);

    seL4_Send(sysCallEndPoint , tag);
    while(1){} //  we never return from Abort
}

static long sys_exit(va_list args)
{
    Abort();
    return 0;
}


static long sys_getpid(va_list args)
{
    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 2);

    seL4_SetMR(0, __SOFA_NR_getpid);
    seL4_SetMR(1 , 0); // useless?

    tag = seL4_Call(sysCallEndPoint, tag);

    if(seL4_MessageInfo_get_length(tag) != 2)
    {
        return -1; // no posix compliant ; getpid should no return any error
    }

    msg = seL4_GetMR(1);

    return (pid_t)msg;
}

static long sys_getppid(va_list args)
{
    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 2);

    seL4_SetMR(0, __SOFA_NR_getppid);
    seL4_SetMR(1 , 0); // useless?

    tag = seL4_Call(sysCallEndPoint, tag); 

    if(seL4_MessageInfo_get_length(tag) != 2)
    {
        return -1; // no posix compliant ; getppid should no return any error
    }

    msg = seL4_GetMR(1);

    return (pid_t)msg;

}
