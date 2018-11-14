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

#include <fcntl.h>
#include <sys/resource.h>
#include <SysCallNum.h>
#include <arch_stdio.h>

static long doRead(int fd, void *buf, size_t count , int expectedNodeType);
static long sys_read(va_list args);
static long sys_write(va_list args);
static long sys_open(va_list args);
static long sys_close(va_list args);

static long sys_nanosleep(va_list args);
static long sys_getpid(va_list args);
static long sys_getppid(va_list args);
static long sys_exit(va_list args);
static long sys_kill(va_list args);
static long sys_wait4(va_list args);
static long sys_execve(va_list args);
static long sys_setpriority(va_list args);
static long sys_getpriority(va_list args);

static long sys_lseek(va_list args);


static long sys_gettimeofday(va_list args);
static long sys_clockgettime(va_list args);

static long sys_getcwd(va_list args);

static long sys_chdir(va_list args);

static long sys_fcntl(va_list args);
static long sys_getdents64(va_list args);

static size_t _Sofa_stdio_write(void *data, size_t count);

static seL4_CPtr sysCallEndPoint = 0;




int SysClientInit(int argc , char* argv[] )
{
    sysCallEndPoint = (seL4_CPtr) atol(argv[0]);

    if (!sysCallEndPoint)
    {
        return 1;
    }
    muslcsys_install_syscall(__NR_read,  sys_read);
    muslcsys_install_syscall(__NR_write, sys_write);
    muslcsys_install_syscall(__NR_open,  sys_open);
    muslcsys_install_syscall(__NR_close, sys_close);


    muslcsys_install_syscall(__NR_nanosleep ,  sys_nanosleep);
    muslcsys_install_syscall(__NR_getpid ,     sys_getpid);
    muslcsys_install_syscall(__NR_getppid ,    sys_getppid);
    muslcsys_install_syscall(__NR_exit,        sys_exit);
    muslcsys_install_syscall(__NR_kill,        sys_kill);
    muslcsys_install_syscall(__NR_execve,      sys_execve);
    muslcsys_install_syscall(__NR_wait4,       sys_wait4);
    muslcsys_install_syscall(__NR_setpriority, sys_setpriority);
    muslcsys_install_syscall(__NR_getpriority, sys_getpriority);

    muslcsys_install_syscall(__NR_lseek	     , sys_lseek);

    muslcsys_install_syscall(__NR_gettimeofday   , sys_gettimeofday);
    muslcsys_install_syscall(__NR_clock_gettime  , sys_clockgettime);

    muslcsys_install_syscall(__NR_getcwd , sys_getcwd);
    muslcsys_install_syscall(__NR_chdir , sys_chdir);


    muslcsys_install_syscall(__NR_fcntl      , sys_fcntl);
    muslcsys_install_syscall(__NR_getdents64 , sys_getdents64);

//    sel4muslcsys_register_stdio_write_fn(_Sofa_stdio_write);
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

static void Abort( int status)
{
    printf("Abort called with status %i\n" , status);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, __SOFA_NR_exit);
    seL4_SetMR(1 , status);
    seL4_Send(sysCallEndPoint , tag);
    while(1){} //  we never return from Abort
}

static long sys_exit(va_list args)
{
    int status =  va_arg(args,int);
    Abort( status);
    return 0;
}


static long sys_getpid(va_list args)
{
    seL4_MessageInfo_t tag;
    seL4_Word msg;

    tag = seL4_MessageInfo_new(0, 0, 0, 2);
    IPCMessageReset();

    IPCPushWord( __SOFA_NR_getpid);
    IPCPushWord( 0);// useless?
//    seL4_SetMR(0, __SOFA_NR_getpid);
//    seL4_SetMR(1 , 0); // useless?

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

static long doRead(int fd, void *buf, size_t count , int expectedNodeType)
{
	if (fd <0)
        {
                return -EBADF;
        }
//      printf("Read request fd %i count %lu\n", fd , count);

        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);
        seL4_SetMR(0, __SOFA_NR_read);
        seL4_SetMR(1, fd);
        seL4_SetMR(2, count);
	seL4_SetMR(3, expectedNodeType);

        tag = seL4_Call(sysCallEndPoint, tag);

	assert(seL4_MessageInfo_get_length(tag) >= 2);

        assert(seL4_GetMR(0) == __SOFA_NR_read);


        ssize_t ret = seL4_GetMR(1);// seL4_MessageInfo_get_length(tag) - 2;

	if (ret > 0)
	{
        	char* b = (char*) buf;
        	for(int i= 0; i<ret;++i)
        	{
                	b[i] = seL4_GetMR(2+i);
        	}
        	b[ret] = 0;
	}

        return ret;
}
// ssize_t read(int fd, void *buf, size_t count);
static long sys_read(va_list args)
{
	const int fd    = va_arg(args , int);
	void*     buf   = va_arg(args , void*);
	size_t    count = va_arg(args , size_t);
	return doRead(fd , buf, count , 1);
/*
	if (fd <0)
	{
		return -EBADF;
	}
//	printf("Read request fd %i count %lu\n", fd , count);

	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
	seL4_SetMR(0, __SOFA_NR_read);
	seL4_SetMR(1, fd);
	seL4_SetMR(2, count);

	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_read);

//	printf("Read returned %i args \n", seL4_MessageInfo_get_length(tag));

	size_t ret = seL4_MessageInfo_get_length(tag) - 2;

	char* b = (char*) buf;
	for(int i= 0; i<ret;++i)
	{
		b[i] = seL4_GetMR(2+i);
	}
	b[ret] = 0;
	return ret;
*/
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
	if(strlen(pathname) == 0)
	{
		return -ENOENT;
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

static long sys_close(va_list args)
{
	int fd  = va_arg(args, int);
	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0, __SOFA_NR_close);
	seL4_SetMR(1, fd);

	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_close);

        return seL4_GetMR(1);
}

// off_t lseek(int fd, off_t offset, int whence);
static long sys_lseek(va_list args)
{
	int fd  = va_arg(args, int);
	off_t offset = va_arg(args, off_t);

	int whence  = va_arg(args, int);

	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);

	seL4_SetMR(0, __SOFA_NR_lseek);
	seL4_SetMR(1, fd);
	seL4_SetMR(2, offset);
	seL4_SetMR(3 , whence);


	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_lseek);
        seL4_Word msg = seL4_GetMR(1);

        return msg;

}


static long sys_write(va_list args)
{
	int fd  = va_arg(args, int);
	const char *buf =  va_arg(args, char*);
	size_t count = va_arg(args, size_t);

	// 0 : sysNum
	// 1 : fd
	// 2 : size
	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3 + count);
	seL4_SetMR(0, __SOFA_NR_write);
	seL4_SetMR(1, fd);
	seL4_SetMR(2, count);
	
	for(int i =0;i<count;++i)
	{
		seL4_SetMR(3+i , buf[i]);
	}

	tag = seL4_Call(sysCallEndPoint, tag);
	assert(seL4_GetMR(0) == __SOFA_NR_write);

	return seL4_GetMR(1);
}


static size_t _Sofa_stdio_write(void *data, size_t count)
{
	return 0;
}

// int gettimeofday(struct timeval *tv, struct timezone *tz);
static long sys_gettimeofday(va_list args)
{
	struct timeval *tv  = va_arg (args, struct timeval* );
	struct timezone *tz = va_arg (args, struct timezone* );

	printf("gettimeofday req\n");
	return 0;
}


// int clock_gettime(clockid_t clk_id, struct timespec *tp);
static long sys_clockgettime(va_list args)
{
	const clockid_t clk_id = va_arg (args, clockid_t);


	if (clk_id <0)
	{
		return -EINVAL;
	}

	struct timespec *tp    = va_arg (args, struct timespec* );

	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0, __SOFA_NR_clock_gettime );
	seL4_SetMR(1, clk_id);

	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_clock_gettime );
	
	seL4_Word retNS = seL4_GetMR(1);

	if (retNS > 0)
	{
		tp->tv_sec  = retNS / 1000000000;
		tp->tv_nsec = retNS - (tp->tv_sec  * 1000000000);

		return 0;
	}

        return retNS;
}

// char *getcwd(char *buf, size_t size);
static long sys_getcwd(va_list args)
{
	char* buf   = va_arg (args, char*);
	size_t size = va_arg (args, size_t);


	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0, __SOFA_NR_getcwd );
	seL4_SetMR(1, size);

	tag = seL4_Call(sysCallEndPoint, tag);
	assert(seL4_GetMR(0) == __SOFA_NR_getcwd );

	ssize_t ret = seL4_GetMR(1);

	if (ret>0)
	{
	    for(int i=0;i<ret;i++)
	    {
	    	    buf[i] = seL4_GetMR(2+i);
	    }
	}

	return ret;
}

// int chdir(const char *path);
static long sys_chdir(va_list args)
{
	const char* path   = va_arg (args, char*);

	if (path == NULL)
	{
		return -EFAULT;
	}
	const size_t strSize = strlen(path);

	if (strSize == 0)
	{
		return -ENOENT;
	}

	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2 + strSize );
	seL4_SetMR(0, __SOFA_NR_chdir );
        seL4_SetMR(1, strSize);

	for(int i=0;i<strSize ;i++)
	{
		seL4_SetMR(2 + i, path[i] );
	}

	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_chdir );
	return seL4_GetMR(1);
}


// int fcntl(int fd, int cmd, ... /* arg */ );
static long sys_fcntl(va_list args)
{
	return 0;
	const int fd  = va_arg (args, int);
	const int cmd = va_arg (args, int);
	
	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);

	seL4_SetMR(0, __SOFA_NR_fcntl);
	seL4_SetMR(1, fd);
	seL4_SetMR(2, cmd);

	switch(cmd)
	{
		case F_SETFD:
		{
			int flag = va_arg (args, int);
			seL4_SetMR(3, flag);
			printf("sys_fcntl flag %i\n" , flag);
		}
		break;

		default:
		return -ENOSYS;
	}

	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_fcntl );
        return seL4_GetMR(1);

}

//int getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count);
static long sys_getdents64(va_list args)
{
	const int fd  	  	    = va_arg (args, int);
	//struct linux_dirent64 *dirp = va_arg (args, struct linux_dirent64 *);
	struct dirent64 *dirp = va_arg (args, struct dirent64 *);
	
	unsigned int count	    = va_arg (args,unsigned int);

	memset(dirp , 0 , count);

//	printf("sys_getdents64 fd %i count %i\n" , fd , count);

/*
	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3); // syscallnum fd reqtype
	seL4_SetMR(0, __SOFA_NR_getdents64);
	seL4_SetMR(1, fd);
	seL4_SetMR(2, 1); // get children count

	tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_getdents64 );

	assert(seL4_GetMR(1) == 1);

	printf("Returned %i\n" , seL4_GetMR(2));
	return -ENOSYS;
*/
	return doRead(fd , dirp, count , 2);
}
