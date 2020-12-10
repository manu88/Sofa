#include "NetService.h"
#include "NameServer.h"
#include "KThread.h"
#include "Environ.h"
#include "Log.h"


static KThread _netServiceThread;
static Service _netService;
static char _netName[] = "NET";

int NetServiceInit()
{
    int error = 0;
    ServiceInit(&_netService, getKernelTaskProcess() );
    _netService.name = _netName;

    vka_object_t ep = {0};
    error = vka_alloc_endpoint(&getKernelTaskContext()->vka, &ep);
    assert(error == 0);
    _netService.baseEndpoint = ep.cptr;
    NameServerRegister(&_netService);
    return 0;
}


static int mainNet(KThread* thread, void *arg)
{
    KernelTaskContext* env = getKernelTaskContext();

    KLOG_INFO("Net Thread started\n");

    while (1)
    {
    }
    
}


int NetServiceStart()
{
    KLOG_INFO("--> Start VFSD thread\n");
    KThreadInit(&_netServiceThread);
    _netServiceThread.mainFunction = mainNet;
    _netServiceThread.name = "NetD";
    int error = KThreadRun(&_netServiceThread, 254, NULL);

    return error;
}