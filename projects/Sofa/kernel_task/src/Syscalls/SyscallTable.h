#pragma once
#include "../Process.h"
#include "../test.h"

void Syscall_sleep(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);
void Syscall_spawn(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);


void Syscall_ThreadNew(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);
void Syscall_ThreadExit(driver_env_t *env, Thread* caller, seL4_MessageInfo_t info);