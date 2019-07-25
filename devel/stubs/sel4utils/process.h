

#pragma once
#include <sel4/types.h>
#include <vka/object.h>
#include <simple/simple.h>


typedef struct
{
    vspace_t vspace;
} sel4utils_process_t;

typedef struct{} sel4utils_process_config_t;


static inline sel4utils_process_config_t process_config_default_simple( simple_t*s, const char* name, int prio)
{
    sel4utils_process_config_t t;
    
    return t;
}


static inline sel4utils_process_config_t process_config_fault_cptr(sel4utils_process_config_t config, seL4_CPtr fault_ep)
{
    return config;
}
