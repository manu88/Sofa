#pragma once
#include <sel4/sel4.h>

void sc_exit(seL4_CPtr endpoint, int code);


int sc_sleep(seL4_CPtr endpoint, int ms);
