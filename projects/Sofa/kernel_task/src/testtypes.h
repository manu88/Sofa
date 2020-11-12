#pragma once

#include "testtypes.h"
#include "Process.h"

#define UNTYPEDS_PER_PROCESS_BASE 2
int process_set_up(driver_env_t *env, uint8_t* untyped_size_bits_list, Process* process, const char* imgName, seL4_Word badge);
void process_run(const char *name, driver_env_t *env, Process* process);
void process_tear_down(driver_env_t *env, Process* process);