#pragma once

//#include <musllibc/syscall.h>
#include <muslcsys/vsyscall.h>


// Numbers must be contigeous as their serve as array indexes!


#define __SOFA_NR_read                  1
#define __SOFA_NR_write                 2
#define __SOFA_NR_open                  3
#define __SOFA_NR_close                 4
#define __SOFA_NR_nanosleep		5
#define __SOFA_NR_getpid		6
#define __SOFA_NR_getppid		7
#define __SOFA_NR_exit			8
#define __SOFA_NR_kill			9
#define __SOFA_NR_execve		10
#define __SOFA_NR_wait4			11

#define __SOFA_NR_setpriority           12
#define __SOFA_NR_getpriority           13
#define __SOFA_NR_lseek			14
