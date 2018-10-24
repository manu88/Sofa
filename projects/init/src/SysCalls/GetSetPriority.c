#include "../SysCalls.h"

int handle_getpriority(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	return 0;
}

int handle_setpriority(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{ 
        return 0;
}
