#pragma once
#include <sel4/sel4.h>
#include <sel4utils/process.h>

seL4_CPtr
process_copy_cap_into(sel4utils_process_t *process, seL4_Word cap_badge, vka_t *parent_vka, seL4_CPtr cap, seL4_CapRights_t rights);

void
sel4osapi_util_copy_cap(
        vka_t *vka, seL4_CPtr src, seL4_CPtr *dest_out);