#pragma once

#include "testtypes.h"
#include "Process.h"

#define UNTYPEDS_PER_PROCESS_BASE 1

void spawnApp(Process* p, const char* imgName, Process* parent);

int process_set_up(uint8_t* untyped_size_bits_list, Process* process, const char* imgName, seL4_Word badge);
void process_run(const char *name, Process* process);
void process_tear_down(Process* process);

void cleanAndRemoveProcess(Process* process, int retCode);