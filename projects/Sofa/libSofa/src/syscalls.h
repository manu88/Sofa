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
#include <sel4/sel4.h>
#include <sys/types.h> // pid_t


// program exit
void sc_exit(seL4_CPtr endpoint, int code);

// sleep syscall
int sc_sleep(seL4_CPtr endpoint, int ms);

int sc_spawn(seL4_CPtr endpoint, uint8_t* ipcBuffer, const char* path);

int sc_wait(seL4_CPtr endpoint, pid_t pid, int *wstatus, int options);

ssize_t sc_write(seL4_CPtr endpoint, const char* data, size_t dataSize);

ssize_t sc_read(seL4_CPtr endpoint, char* data, size_t dataSize, char until);

void sc_debug(seL4_CPtr endpoint, SofaDebugCode code);

pid_t sc_getppid(seL4_CPtr endpoint);


seL4_CPtr sc_regservice(seL4_CPtr endpoint, const char* serviceName, int *err);
seL4_CPtr sc_getservice(seL4_CPtr endpoint, const char* serviceName, int *err);

int sc_reboot(seL4_CPtr endoint, int code);

long sc_mmap(seL4_CPtr endpoint, void *addr, size_t length, int prot, int flags, int fd, off_t offset);
long sc_munmap(seL4_CPtr endpoint, void* addr, size_t length);