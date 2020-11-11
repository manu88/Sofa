#pragma once

#include "testtypes.h"
#include "Process.h"


void basic_set_up(driver_env_t env, Process* process);
void basic_run_test(const char *name, driver_env_t env, Process* process);
void basic_tear_down(driver_env_t env, Process* process);