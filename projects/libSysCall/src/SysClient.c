
#include <stdlib.h>
#include <muslcsys/vsyscall.h>

#include "SysCallsList.h"
#include "SysClient.h"


static size_t _Sofa_stdio_write(void *data, size_t count);

// defined as extern in SysCallsList.h
seL4_CPtr sysCallEndPoint = 0;


int SysClientInit(int argc , char* argv[] )
{
    sysCallEndPoint = (seL4_CPtr) atol(argv[0]);

    if (!sysCallEndPoint)
    {
        return 1;
    }
    muslcsys_install_syscall(__NR_read,  sofa_read);
    muslcsys_install_syscall(__NR_write, sofa_write);
    muslcsys_install_syscall(__NR_open,  sofa_open);
    muslcsys_install_syscall(__NR_close, sofa_close);
    muslcsys_install_syscall(__NR_stat,  sofa_stat);

    muslcsys_install_syscall(__NR_nanosleep ,  sofa_nanosleep);
    muslcsys_install_syscall(__NR_getpid ,     sofa_getpid);
    muslcsys_install_syscall(__NR_getppid ,    sofa_getppid);
    muslcsys_install_syscall(__NR_exit,        sofa_exit);
    muslcsys_install_syscall(__NR_kill,        sofa_kill);
    muslcsys_install_syscall(__NR_execve,      sofa_execve);
    muslcsys_install_syscall(__NR_wait4,       sofa_wait4);
    muslcsys_install_syscall(__NR_setpriority, sofa_setpriority);
    muslcsys_install_syscall(__NR_getpriority, sofa_getpriority);

    muslcsys_install_syscall(__NR_lseek	     , sofa_lseek);

    muslcsys_install_syscall(__NR_gettimeofday   , sofa_gettimeofday);
    muslcsys_install_syscall(__NR_clock_gettime  , sofa_clockgettime);

    muslcsys_install_syscall(__NR_getcwd , sofa_getcwd);
    muslcsys_install_syscall(__NR_chdir ,  sofa_chdir);


    muslcsys_install_syscall(__NR_fcntl      , sofa_fcntl);
    muslcsys_install_syscall(__NR_getdents64 , sofa_getdents64);

    muslcsys_install_syscall(__NR_mkdir	     , sofa_mkdir);

//    muslcsys_install_syscall(__NR_writev , sys_writev);

//    sel4muslcsys_register_stdio_write_fn(_Sofa_stdio_write);
    return 0;
}

static size_t _Sofa_stdio_write(void *data, size_t count)
{
	return 0;
}


void DebugDumpScheduler()
{
	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2 );

        seL4_SetMR(0, __SOFA_NR_debugSys);
        seL4_SetMR(1,  1);
	seL4_Send(sysCallEndPoint , tag);
}
