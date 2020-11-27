#pragma once
#include "../Process.h"
#include "../Environ.h"
#include "../Log.h"


typedef void (*SyscallMethod)(Thread* caller, seL4_MessageInfo_t info);

void Syscall_exit(Thread* caller, seL4_MessageInfo_t info);
void Syscall_sleep(Thread* caller, seL4_MessageInfo_t info);

void Syscall_spawn(Thread* caller, seL4_MessageInfo_t info);
void Syscall_wait(Thread* caller, seL4_MessageInfo_t info);
void Syscall_Kill(Thread* caller, seL4_MessageInfo_t info);

void Syscall_ThreadNew(Thread* caller, seL4_MessageInfo_t info);
void Syscall_ThreadExit(Thread* caller, seL4_MessageInfo_t info);


void Syscall_Read(Thread* caller, seL4_MessageInfo_t info);
void Syscall_Write(Thread* caller, seL4_MessageInfo_t info);

void Syscall_PPID(Thread* caller, seL4_MessageInfo_t info);
void Syscall_Debug(Thread* caller, seL4_MessageInfo_t info);
void Syscall_RequestCap(Thread* caller, seL4_MessageInfo_t info);

void Syscall_RegisterService(Thread* caller, seL4_MessageInfo_t info);

static SyscallMethod syscallTable[] =
{
    NULL,

    Syscall_ThreadNew,
    Syscall_ThreadExit,

    Syscall_exit,

    Syscall_sleep,

    Syscall_spawn,
    Syscall_wait,
    Syscall_Kill,

    Syscall_Read,
    Syscall_Write,
    Syscall_PPID,
    Syscall_Debug,
    Syscall_RequestCap,
    Syscall_RegisterService,
};


static inline void Syscall_perform(int rpcID, Thread* caller, seL4_MessageInfo_t info)
{
    //LOG_TRACE("--> Syscall %i\n", rpcID);
    syscallTable[rpcID](caller, info);
}