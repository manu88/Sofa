#pragma once
#include <sel4/sel4.h>

// program exit
void sc_exit(seL4_CPtr endpoint, int code);

// sleep syscall
int sc_sleep(seL4_CPtr endpoint, int ms);
