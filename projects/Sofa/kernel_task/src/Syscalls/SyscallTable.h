#pragma once
#include "../Process.h"
#include "../Environ.h"



typedef void (*SyscallMethod)(Thread* caller, seL4_MessageInfo_t info);

void Syscall_exit(Thread* caller, seL4_MessageInfo_t info);
void Syscall_sleep(Thread* caller, seL4_MessageInfo_t info);

void Syscall_spawn(Thread* caller, seL4_MessageInfo_t info);
void Syscall_wait(Thread* caller, seL4_MessageInfo_t info);


void Syscall_ThreadNew(Thread* caller, seL4_MessageInfo_t info);
void Syscall_ThreadExit(Thread* caller, seL4_MessageInfo_t info);


void Syscall_read(Thread* caller, seL4_MessageInfo_t info);

static SyscallMethod syscallTable[] =
{
    NULL,

    Syscall_ThreadNew,
    Syscall_ThreadExit,

    Syscall_exit,

    Syscall_sleep,

    Syscall_spawn,
    Syscall_wait,

    Syscall_read,
};