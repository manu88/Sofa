#include <runtime.h>
#include <Sofa.h>

void sc_exit(seL4_CPtr endpoint, int code)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Exit);
    seL4_SetMR(1, code);
    seL4_Send(endpoint, info);

}