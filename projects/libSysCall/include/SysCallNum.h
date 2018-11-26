/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

//#include <musllibc/syscall.h>
//#include <muslcsys/vsyscall.h>


// Numbers must be contigeous as their serve as array indexes!


#define __SOFA_NR_debugSys		1
#define __SOFA_NR_read                  2
#define __SOFA_NR_write                 3
#define __SOFA_NR_open                  4
#define __SOFA_NR_close                 5
#define __SOFA_NR_stat			6
#define __SOFA_NR_nanosleep		7
#define __SOFA_NR_getpid		8
#define __SOFA_NR_getppid		9
#define __SOFA_NR_exit			10
#define __SOFA_NR_kill			11
#define __SOFA_NR_execve		12
#define __SOFA_NR_wait4			13

#define __SOFA_NR_setpriority           14
#define __SOFA_NR_getpriority           15
#define __SOFA_NR_lseek			16

#define __SOFA_NR_gettimeofday          17
#define __SOFA_NR_clock_gettime         18
#define __SOFA_NR_getcwd		19
#define __SOFA_NR_chdir                 20
#define __SOFA_NR_fcntl			21
#define __SOFA_NR_mkdir 		22

