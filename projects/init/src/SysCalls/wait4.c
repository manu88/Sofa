#include "../Utils.h"
#include "../SysCalls.h"
#include "../ProcessDef.h"

int handle_wait4(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    const pid_t pidToWait = seL4_GetMR(1);
    const int options     = seL4_GetMR(2);

    // check if senderProcess has any children

    const int childrenCount = ProcessGetNumChildren( senderProcess);

    // no child to wait -> reply immediatly
    if(childrenCount == 0)
    {
	    seL4_SetMR(1, -ECHILD );
	    seL4_Reply( message );
	    return 0;
    }


    printf("Process %i has %i child to wait! (req pid %i) \n",senderProcess->_pid,childrenCount , pidToWait);

    Process* processToWait = ProcessGetChildByPID(senderProcess , pidToWait);

    assert(processToWait);

    WaiterListEntry* waiter = malloc(sizeof(WaiterListEntry) );
    
    if(!waiter)
    {
	seL4_SetMR(1, -ENOMEM);
	seL4_Reply( message );
    }

    waiter->process = senderProcess;
    waiter->reply = get_free_slot(context); 
    int error = cnode_savecaller( context, waiter->reply );

    ProcessRegisterWaiter(processToWait, waiter);
    return 0;
}
