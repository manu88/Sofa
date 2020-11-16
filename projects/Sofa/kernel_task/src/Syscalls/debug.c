#include <Sofa.h>
#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"
#include "../Panic.h"

void Syscall_Debug(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();

    Process* p = NULL;
    FOR_EACH_PROCESS(p)
    {
        printf("%i %s\n", ProcessGetPID(p), ProcessGetName(p));
    }

}