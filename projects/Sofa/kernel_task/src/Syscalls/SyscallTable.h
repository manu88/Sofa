/*
 * This file is part of the Sofa project
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
#include "ProcessList.h"
#include "Environ.h"
#include "Log.h"
#include <Sofa.h>


typedef void (*SyscallMethod)(Thread* caller, seL4_MessageInfo_t info);

void Syscall_exit(Thread* caller, seL4_MessageInfo_t info);
void Syscall_sleep(Thread* caller, seL4_MessageInfo_t info);


void Syscall_ThreadNew(Thread* caller, seL4_MessageInfo_t info);
void Syscall_ThreadExit(Thread* caller, seL4_MessageInfo_t info);


void Syscall_Read(Thread* caller, seL4_MessageInfo_t info);
void Syscall_Write(Thread* caller, seL4_MessageInfo_t info);

void Syscall_PPID(Thread* caller, seL4_MessageInfo_t info);
void Syscall_Debug(Thread* caller, seL4_MessageInfo_t info);

void Syscall_RequestCap(Thread* caller, seL4_MessageInfo_t info);

void Syscall_RegisterService(Thread* caller, seL4_MessageInfo_t info);
void Syscall_GetService(Thread* caller, seL4_MessageInfo_t info);

void SysCall_Reboot(Thread* caller, seL4_MessageInfo_t info);

void Syscall_mmap(Thread* caller, seL4_MessageInfo_t info);
void Syscall_munmap(Thread* caller, seL4_MessageInfo_t info);

void Syscall_shareMem(Thread* caller, seL4_MessageInfo_t info);

static SyscallMethod syscallTable[] =
{
    NULL,

    Syscall_ThreadNew,
    Syscall_ThreadExit,

    Syscall_exit,

    Syscall_sleep,

    Syscall_mmap,
    Syscall_munmap,

    Syscall_shareMem,

    Syscall_Read,
    Syscall_Write,
    Syscall_PPID,
    Syscall_Debug,
    Syscall_RequestCap,
    Syscall_RegisterService,
    Syscall_GetService,
    SysCall_Reboot,
};


static inline void Syscall_perform(int rpcID, Thread* caller, seL4_MessageInfo_t info)
{
    //LOG_TRACE("--> Syscall %i\n", rpcID);
    syscallTable[rpcID](caller, info);
}