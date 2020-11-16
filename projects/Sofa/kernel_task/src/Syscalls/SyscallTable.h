#pragma once
#include "../Process.h"
#include "../test.h"



typedef void (*SyscallMethod)(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);

void Syscall_exit(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);
void Syscall_sleep(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);

void Syscall_spawn(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);
void Syscall_wait(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);


void Syscall_ThreadNew(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);
void Syscall_ThreadExit(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);


void Syscall_read(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);

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