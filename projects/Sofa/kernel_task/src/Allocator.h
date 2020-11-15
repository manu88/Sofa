#pragma once

#include "test.h"


int findUntypedBySize(driver_env_t* env, size_t size);
size_t countUntypedBySize(driver_env_t* env, size_t size, uint8_t* size_bit_list, size_t list_size);