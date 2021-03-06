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
#include <Sofa.h>

// program exit
void sc_exit(seL4_CPtr endpoint, int code);

// sleep syscall
int sc_sleep(seL4_CPtr endpoint, int ms);

// get current time in nanoseconds
uint64_t sc_gettime(seL4_CPtr endpoint);


void sc_debug(seL4_CPtr endpoint, SofaDebugCode code);

pid_t sc_getppid(seL4_CPtr endpoint);


seL4_CPtr sc_regservice(seL4_CPtr endpoint, const char* serviceName, int *err);
seL4_CPtr sc_getservice(seL4_CPtr endpoint, const char* serviceName, int *err);

int sc_reboot(seL4_CPtr endoint, int code);

long sc_mmap(seL4_CPtr endpoint, void *addr, size_t length, int prot, int flags, int fd, off_t offset);
long sc_munmap(seL4_CPtr endpoint, void* addr, size_t length);

long sc_sharemem(seL4_CPtr endpoint, void* addr, seL4_Word with, uint64_t rights);

long sc_caprequest(seL4_CPtr endoint, CapRequest type);

typedef struct
{
    seL4_CPtr tcb;
    seL4_CPtr ep;
    seL4_CPtr ipcBuf;
    seL4_CPtr ipcBufAddr;
    void* stackTop;

    void* sofaIPC;
} ThreadConfig;


int sc_newThread(seL4_CPtr endpoint, ThreadConfig* conf);