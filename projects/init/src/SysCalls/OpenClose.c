#include "../SysCalls.h"
#include <SysCallNum.h>


int handle_read(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	return 0;
}

int handle_write(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        return 0;
}


int handle_open(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

	const int msgLen = seL4_MessageInfo_get_length(message);
	printf("handle_open msg len %i\n" , msgLen);
	assert(msgLen > 2);

	char* pathname = malloc(sizeof(char)*msgLen -1);
	if(!pathname)
	{
		// return some error
	}

	int flags = seL4_GetMR(1);

	for(int i=0;i<msgLen-1;++i)
        {
            pathname[i] =  (char) seL4_GetMR(2+i);
        }

	pathname[msgLen] = '0';

	int ret= -ENOSYS;

	message = seL4_MessageInfo_new(0, 0, 0, 2);

	seL4_SetMR(0, __SOFA_NR_open );
	seL4_SetMR(1, ret);

	printf("Got handle_open '%s' %i\n" , pathname,flags);


	seL4_Reply( message );


	free(pathname);
        return 0;
}


int handle_close(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        return 0;
}


