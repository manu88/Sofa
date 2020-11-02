#pragma once
#include "proc_ctx.h"


int is_slot_empty(ProcessContext* context, seL4_Word slot);
seL4_Word get_free_slot(ProcessContext* context);

int cnode_mint(ProcessContext* context, seL4_CPtr src, seL4_CPtr dest, seL4_CapRights_t rights, seL4_Word badge);


int cnode_move(ProcessContext* context, seL4_CPtr src, seL4_CPtr dest);