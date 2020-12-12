#include <Sofa.h>
#include "SyscallTable.h"
#include "testtypes.h"
#include "utils.h"
#include "Panic.h"
#include "NameServer.h"
#include "DeviceTree.h"

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
    Process* proc = caller->_base.process;

    SofaDebugCode op = seL4_GetMR(1);
    switch (op)
    {
    case SofaDebugCode_DumpSched:
        seL4_DebugDumpScheduler();
        break;
    case SofaDebugCode_ListDevices:
    {
        IODevice* dev = NULL;
        printf("Start device list\n");
        FOR_EACH_DEVICE(dev)
        {
            printf("'%s' type %i\n", dev->name, dev->type);
        }
    }    
        break;
    case SofaDebugCode_ListServices:
    {
        Service* s = NULL;
        Service* tmp = NULL;
        FOR_EACH_SERVICE(s, tmp)
        {
            printf("'%s' owner %i %s\n", s->name, ProcessGetPID(s->owner), ProcessGetName(s->owner));
        }
    }
        break;
    case SofaDebugCode_ListProcesses:
        {
            Process* p = NULL;
            FOR_EACH_PROCESS(p)
            {
                printf("%i ", ProcessGetPID(p));
                printf("%c ", getProcessStateStr(p->state));
                if(p->argv && p->argc)
                {
                    for(int i=0;i<p->argc;i++)
                    {
                        printf("%s ", p->argv[i]);
                    }
                }
                else
                {
                    printf("%s ", ProcessGetName(p));
                }
                
                printf("%s", proc == p ? "*":"");

                printf("\n");
            }

        }
        break;
    
    default:
        break;
    }


}