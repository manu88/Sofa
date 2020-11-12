#pragma once
#include "../Process.h"
#include "../test.h"

void Syscall_sleep(driver_env_t *env, Process* callingProcess, seL4_MessageInfo_t info);