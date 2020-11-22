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

static void doListProcesses(Process* caller)
{
    Process* p = NULL;
    FOR_EACH_PROCESS(p)
    {
        printf("%i %s %c %s\n", ProcessGetPID(p), ProcessGetName(p), getProcessStateStr(p->state), p == caller ? "*":"");
    }
}


static void doMemStats()
{
    printf("Alloc pages: %zi\n", getNumPagesAlloc());
}
void Syscall_Debug(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* proc = caller->process;
    SofaDebugCode op = seL4_GetMR(1);
    switch (op)
    {
    case SofaDebugCode_ListProcesses:
        doListProcesses(proc);
        break;

    case SofaDebugCode_MemStats:
        doMemStats();
        break;
    
    default:
        printf("[Syscall_Debug] Unknown debug op code %i\n", op);
        break;
    }



}