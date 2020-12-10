#include "NetService.h"
#include "NameServer.h"
#include "KThread.h"
#include "Environ.h"
#include "Log.h"
#include "Process.h"
#include <Sofa.h>
#include <utils/uthash.h>


typedef struct
{
    ThreadBase* caller;
    UT_hash_handle hh; /* makes this structure hashable */
    char* buff;


}Client;


static Client* _clients = NULL;

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


static void _Bind(ThreadBase* caller, seL4_MessageInfo_t msg)
{
/*
    seL4_SetMR(1, familly);
    seL4_SetMR(2, protoc);
    seL4_SetMR(3, port);
*/
    KLOG_INFO("Net Bind message from %i\n", ProcessGetPID(caller->process));
    int familly = seL4_GetMR(1);
    int protoc = seL4_GetMR(2);
    int port = seL4_GetMR(3);
    KLOG_INFO("fam=%i protoc=%i port=%i\n", familly, protoc, port);

    seL4_Reply(msg);

}


static void _Register(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    KLOG_INFO("Net Register message from %i\n", ProcessGetPID(caller->process));
    KernelTaskContext* env = getKernelTaskContext();
    char* buff = vspace_new_pages(&env->vspace, seL4_ReadWrite, 1, PAGE_BITS_4K);
    assert(buff);
    void* buffShared = vspace_share_mem(&env->vspace,
                                        &caller->process->native.vspace,
                                        buff,
                                        1,
                                        PAGE_BITS_4K,
                                        seL4_ReadWrite,
                                        1
                                        );
    assert(buffShared);
    Client* client = malloc(sizeof(Client));
    assert(client);
    memset(client, 0, sizeof(Client));
    client->caller = caller;
    client->buff = buff;
    HASH_ADD_PTR(_clients, caller, client);
    seL4_SetMR(1, buffShared);
    seL4_Reply(msg);

}

static int mainNet(KThread* thread, void *arg)
{
    KernelTaskContext* env = getKernelTaskContext();

    KLOG_INFO("Net Thread started\n");

    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(_netService.baseEndpoint, &sender);
        ThreadBase* caller =(ThreadBase*) sender;
        assert(caller->process);

        NetRequest req = (NetRequest) seL4_GetMR(0);
        switch (req)
        {
        case NetRequest_Register:
            _Register(caller, msg);
            break;
        case NetRequest_Bind:
            _Bind(caller, msg);
            break;
        
        default:
            KLOG_TRACE("[NetService] Received unknown code %i from %i\n", req, sender);
            assert(0);
            break;
        }
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