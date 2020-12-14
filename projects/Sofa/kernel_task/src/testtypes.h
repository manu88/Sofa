#pragma once

#include "testtypes.h"
#include "ProcessList.h"

#define UNTYPEDS_PER_PROCESS_BASE 1

void spawnApp(Process* p, const char* imgName, Process* parent);
void doExit(Process*p, int retCode);


int process_set_up(uint8_t* untyped_size_bits_list, Process* process, const char* imgName, seL4_Word badge);
void process_run(const char *name, Process* process);
void process_tear_down(Process* process);

void process_suspend(Process*p);
void process_resume(Process*p);