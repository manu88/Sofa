#pragma once



#include "Bootstrap.h"


seL4_Word get_free_slot( InitContext* context);
int cnode_savecaller( InitContext* context,seL4_CPtr cap);
int cnode_delete( InitContext* context,seL4_CPtr slot);
