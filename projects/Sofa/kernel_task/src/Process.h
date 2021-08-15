/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once


#include "ProcessList.h"

#define UNTYPEDS_PER_PROCESS_BASE 1

#define MAKE_EXIT_CODE(ret, sig) ((ret) << 8 | (sig))

void spawnApp(Process* p, const char* imgName, Process* parent);
void doExit(Process*p, int retCode);

int ProcessCreateSofaIPCBuffer(Process* p, void** addr, void** procAddr);
int process_set_up(uint8_t* untyped_size_bits_list, Process* process, const char* imgName, seL4_Word badge);
void process_run(const char *name, Process* process);
void process_tear_down(Process* process);

void process_suspend(Process*p);
void process_resume(Process*p);

void* process_new_pages(Process*p, seL4_CapRights_t rights, size_t numPages);
void* process_reserve_range(Process* p, size_t bytes, seL4_CapRights_t rights);
void process_unmap_pages(Process*p, void *vaddr, size_t numPages);

int process_handle_vm_fault(Process* p, uintptr_t faultAddr);