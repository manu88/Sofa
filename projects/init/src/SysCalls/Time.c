#include "../SysCalls.h"
#include <SysCallNum.h>
#include <assert.h>


int handle_getTimeOfTheDay(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	return 0;
}


int handle_clock_getTime(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	return 0;
}


