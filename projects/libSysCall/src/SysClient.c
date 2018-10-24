#include "SysClient.h"


#include <unistd.h> // pid_t
#include <stdio.h>
#include <sel4/sel4.h>
#include <stdlib.h> // atol

#include <utils/time.h>
#include <time.h>

#include <muslcsys/vsyscall.h>
#include <assert.h>
static long sys_nanosleep(va_list args);
static long sys_getpid(va_list args);
static long sys_getppid(va_list args);
static long sys_exit(va_list args);

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

    return 0;
}


static long sys_nanosleep(va_list args)
{
    struct timespec *req = va_arg(args, struct timespec*);
    /* construct a sleep call */
    int millis = req->tv_sec * MS_IN_S;
    millis += req->tv_nsec / NS_IN_MS;
    printf("Usleep %i ms\n",millis);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, __NR_nanosleep);
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
    seL4_SetMR(0, __NR_exit);

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

    seL4_SetMR(0, __NR_getpid);
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

    seL4_SetMR(0, __NR_getppid);
    seL4_SetMR(1 , 0); // useless?

    tag = seL4_Call(sysCallEndPoint, tag); 

    if(seL4_MessageInfo_get_length(tag) != 2)
    {
        return -1; // no posix compliant ; getppid should no return any error
    }

    msg = seL4_GetMR(1);

    return (pid_t)msg;

}
