#pragma once
#include <sel4/sel4.h>
#include <sys/types.h> // pid_t


// program exit
void sc_exit(seL4_CPtr endpoint, int code);

// sleep syscall
int sc_sleep(seL4_CPtr endpoint, int ms);

int sc_spawn(seL4_CPtr endpoint, uint8_t* ipcBuffer, const char* path);

int sc_wait(seL4_CPtr endpoint, pid_t pid, int *wstatus, int options);


ssize_t sc_read(seL4_CPtr endpoint, char* data, size_t dataSize, char until);

int sc_kill(seL4_CPtr endpoint, pid_t pid, int sig);

void sc_debug(seL4_CPtr endpoint, SofaDebugCode code);