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

#include <sel4/sel4.h>
#include <SysCallNum.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>

extern seL4_CPtr sysCallEndPoint;

// implemented in read.c
long sofa_read(va_list args);
long sofa_getdents64(va_list args);


long sofa_write(va_list args);


long sofa_open(va_list args);


long sofa_close(va_list args);
long sofa_stat(va_list args);

long sofa_nanosleep(va_list args);
long sofa_getpid(va_list args);
long sofa_getppid(va_list args);
long sofa_exit(va_list args);
long sofa_kill(va_list args);
long sofa_wait4(va_list args);
long sofa_execve(va_list args);
long sofa_setpriority(va_list args);
long sofa_getpriority(va_list args);
long sofa_lseek(va_list args);

long sofa_gettimeofday(va_list args);
long sofa_clockgettime(va_list args);

long sofa_getcwd(va_list args);

long sofa_chdir(va_list args);

long sofa_fcntl(va_list args);

long sofa_mkdir(va_list args);
long sofa_writev(va_list args);
