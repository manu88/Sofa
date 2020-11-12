#pragma once

#include "testtypes.h"
#include "Process.h"

#define UNTYPEDS_PER_PROCESS_BASE 8
int basic_set_up(driver_env_t env, uint8_t* untyped_size_bits_list, Process* process, const char* imgName, seL4_Word badge);
void basic_run_test(const char *name, driver_env_t env, Process* process);
void basic_tear_down(driver_env_t env, Process* process);