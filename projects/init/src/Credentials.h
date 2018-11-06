#pragma once


#include <stdint.h>

#define _KERNEL_CAPABILITY_U64S 1

typedef struct
{
	uint64_t cap[_KERNEL_CAPABILITY_U64S];
} Capacity;

