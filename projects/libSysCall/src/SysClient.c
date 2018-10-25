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

#include <sys/resource.h>
#include <SysCallNum.h>


static long sys_write(va_list args);
static long sys_open(va_list args);


static long sys_nanosleep(va_list args);
static long sys_getpid(va_list args);
static long sys_getppid(va_list args);
static long sys_exit(va_list args);
static long sys_kill(va_list args);
static long sys_wait4(va_list args);
static long sys_execve(va_list args);
static long sys_setpriority(va_list args);
static long sys_getpriority(va_list args);



static seL4_CPtr sysCallEndPoint = 0;

int SysClientInit(int argc , char* argv[] )
{
    sysCallEndPoint = (seL4_CPtr) atol(argv[0]);

    if (!sysCallEndPoint)
    {
        return 1;
    }
    
    muslcsys_install_syscall(__NR_write, sys_write);
    muslcsys_install_syscall(__NR_open, sys_open);

    muslcsys_install_syscall(__NR_nanosleep ,  sys_nanosleep);
    muslcsys_install_syscall(__NR_getpid ,     sys_getpid);
    muslcsys_install_syscall(__NR_getppid ,    sys_getppid);
    muslcsys_install_syscall(__NR_exit,        sys_exit);
    muslcsys_install_syscall(__NR_kill,        sys_kill);
    muslcsys_install_syscall(__NR_execve,      sys_execve);
    muslcsys_install_syscall(__NR_wait4,       sys_wait4);
    muslcsys_install_syscall(__NR_setpriority, sys_setpriority);
    muslcsys_install_syscall(__NR_getpriority, sys_getpriority);

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

    assert(seL4_GetMR(0) == __SOFA_NR_wait4);
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

    assert(seL4_GetMR(0) == __SOFA_NR_execve);
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

    assert(seL4_GetMR(0) == __SOFA_NR_kill);
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

    assert(seL4_GetMR(0) == __SOFA_NR_nanosleep);

    return seL4_GetMR(1);
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

    assert(seL4_GetMR(0) == __SOFA_NR_getpid);
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

    assert(seL4_GetMR(0) == __SOFA_NR_getppid);
    msg = seL4_GetMR(1);

    return (pid_t)msg;

}

//int getpriority(int which, id_t who);
static long sys_getpriority(va_list args)
{
    // The range of the nice value is +19 (low priority) to -20 (high priority)
    int which  = va_arg(args, int);
    id_t who   = va_arg(args, id_t);

    printf("Client ; getpriority %i %i\n",which , who);

    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 3);

    seL4_SetMR(0, __SOFA_NR_getpriority);
    seL4_SetMR(1 , which);
    seL4_SetMR(2 , who);

    tag = seL4_Call(sysCallEndPoint, tag);
    assert(seL4_GetMR(0) == __SOFA_NR_getpriority);
    msg = seL4_GetMR(1);

    return msg;

}

//int setpriority(int which, id_t who, int prio);
static long sys_setpriority(va_list args)
{
    // The range of the nice value is +19 (low priority) to -20 (high priority)
    int which  = va_arg(args, int);
    id_t who   = va_arg(args, id_t);
    int prio   = va_arg(args, int);
    int mappedPrio = (-prio + 19)*6;

    printf("Client ; setpriority %i %i %i mapped %i\n",which , who , prio , mappedPrio);

    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 4);

    seL4_SetMR(0, __SOFA_NR_setpriority);
    seL4_SetMR(1 , which);
    seL4_SetMR(2 , who);
    seL4_SetMR(3 , mappedPrio);

    tag = seL4_Call(sysCallEndPoint, tag);
    assert(seL4_GetMR(0) == __SOFA_NR_setpriority);
    msg = seL4_GetMR(1);

    return msg;

}

//int open(const char *pathname, int flags);
static long sys_open(va_list args)
{
	const char* pathname = va_arg(args,const char*);
	const int flags = va_arg(args, int);


	if(pathname == NULL)
	{
		return -EFAULT;
	}
	seL4_MessageInfo_t tag;
        seL4_Word msg;

        tag = seL4_MessageInfo_new(0, 0, 0, 2 + strlen(pathname));
	seL4_SetMR(0, __SOFA_NR_open);
        seL4_SetMR(1, flags);

	for(int i=0;i<strlen(pathname);++i)
	{
		seL4_SetMR(i+2,pathname[i]);
	}

	tag = seL4_Call(sysCallEndPoint, tag);
	assert(seL4_GetMR(0) == __SOFA_NR_open);
    	msg = seL4_GetMR(1);

    	return msg;

}
static long sys_write(va_list args)
{
	assert(0);
	int fd  = va_arg(args, int);
	const void *buf =  va_arg(args, void*);
	size_t count = va_arg(args, size_t);

	return 0;
}
