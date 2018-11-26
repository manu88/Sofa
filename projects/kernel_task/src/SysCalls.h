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


#include "Bootstrap.h"
#include "ProcessDef.h"

typedef int (*SysCallHandler)(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);


int handle_debugSys(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);

int handle_read(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_write(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_open(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_close(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_stat(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);

int handle_nanosleep(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_getpid(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_getppid(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_exit(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_kill(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_execve(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_wait4(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);


int handle_getpriority(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_setpriority(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);


int handle_lseek(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);


// time 
int handle_getTimeOfTheDay(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_clock_getTime(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);


// dirs

int handle_getcwd(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_chdir(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);

//int handle_getdents64(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_fcntl(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);

int handle_mkdir(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message);
