#include "../SysCalls.h"
#include "../ProcessDef.h"

int handle_wait4(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    const pid_t pidToWait = seL4_GetMR(1);
    const int options     = seL4_GetMR(2);

    // check if senderProcess has any children

    const int childrenCount = ProcessGetNumChildren( senderProcess);

    int retCode = -ECHILD;
    if(childrenCount )
    {

    }

    seL4_SetMR(1, retCode );
    seL4_Reply( message );
        return 0;
}
