#include <Sofa.h>
#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"
#include "../Panic.h"

static char getProcessStateStr(ProcessState s)
{
    switch (s)
    {
    case ProcessState_Running:
        return 'R';
    case ProcessState_Zombie:
        return 'Z';
    
    default:
        assert(0);
    }
}

void Syscall_Debug(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* proc = caller->process;

    Process* p = NULL;
    FOR_EACH_PROCESS(p)
    {
        printf("%i %s %c %s\n", ProcessGetPID(p), ProcessGetName(p), getProcessStateStr(p->state), proc == p ? "*":"");
    }

}