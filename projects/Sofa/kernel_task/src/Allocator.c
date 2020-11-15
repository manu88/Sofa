#include "Allocator.h"

int findUntypedBySize(driver_env_t* env, size_t size)
{
    return -1;
}


size_t countUntypedBySize(driver_env_t* env, size_t size, uint8_t* size_bit_list, size_t list_size)
{
    size_t count = 0;
    for (size_t i=0; i < list_size; i++)
    {
        size_t bitSize = BIT(size_bit_list[i]);
        if(bitSize >= size)
        {
            count++;
        }

    }
    return count;
}