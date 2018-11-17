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

typedef int (*SysCallHandler)(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);


int handle_read(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_write(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_open(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_close(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);

int handle_nanosleep(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_getpid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_getppid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_exit(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_kill(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_execve(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_wait4(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);


int handle_getpriority(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_setpriority(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);


int handle_lseek(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);


// time 
int handle_getTimeOfTheDay(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_clock_getTime(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);


// dirs

int handle_getcwd(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_chdir(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);

//int handle_getdents64(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
int handle_fcntl(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);

int handle_mkdir(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message);
