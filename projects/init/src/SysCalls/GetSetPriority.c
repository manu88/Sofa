#include "../SysCalls.h"
#include <sys/resource.h>
#include <SysCallNum.h>

int handle_getpriority(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int which = seL4_GetMR(1);
	const id_t who  = seL4_GetMR(2);
	// TODO implement me :)
	seL4_SetMR(0 , __SOFA_NR_getpriority);
	seL4_SetMR(1,-ENOSYS);
	seL4_Reply( message );
	return 0;
}

int handle_setpriority(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{ 
	const int which = seL4_GetMR(1);
	const id_t who  = seL4_GetMR(2);
	const int prio  = seL4_GetMR(3);
	
	int error = -ENOSYS;

	if( which == 0 && who == 0)
	{
		printf("Process %i asks to nice itset to %i\n", senderProcess->_pid , prio);
		// the sender nices itself
		error = ProcessSetPriority(context,senderProcess , prio);
	}

	seL4_SetMR(0 , __SOFA_NR_setpriority);
	seL4_SetMR(1,error);
        seL4_Reply( message );
        return 0;
}
