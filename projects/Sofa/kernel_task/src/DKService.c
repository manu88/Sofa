#include "DKService.h"
#include "NameServer.h"
#include "Environ.h"
#include "KThread.h"
#include "Log.h"
#include "Process.h"


static Service _dkService;
static char _dkName[] = "deviceKit";
static KThread _dkThread;

int DKServiceInit()
{
        int error = 0;
    ServiceInit(&_dkService, getKernelTaskProcess() );
    _dkService.name = _dkName;

    ServiceCreateKernelTask(&_dkService);
    NameServerRegister(&_dkService);
    return 0;

}

static int mainDKService(KThread* thread, void *arg)
{
    KLOG_DEBUG("DKService thread running\n");

    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(_dkService.baseEndpoint, &sender);
        ThreadBase* caller =(ThreadBase*) sender;
        assert(caller->process);
        KLOG_DEBUG("DKService request from %i\n", ProcessGetPID(caller->process));
    }
    
}

int DKServiceStart()
{
    KThreadInit(&_dkThread);
    _dkThread.mainFunction = mainDKService;
    _dkThread.name = "DKService";
    int error = KThreadRun(&_dkThread, 254, NULL);

    return error;

}