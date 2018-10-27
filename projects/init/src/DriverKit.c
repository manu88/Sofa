#include "DriverKit.h"

int DriverKitInit(InitContext* context)
{	
//	seL4_CPtr cap = simple_get_IOPort_cap(&context->simple, 1,1);
// cspace_irq_control_get_cap( simple_get_cnode(&context->simple)) , seL4_CapIRQControl, 1);

	return 1;
}
