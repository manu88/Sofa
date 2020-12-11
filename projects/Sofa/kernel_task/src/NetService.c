#include "NetService.h"
#include "NameServer.h"
#include "KThread.h"
#include "Environ.h"
#include "Log.h"
#include "Process.h"
#include <Sofa.h>
#include <utils/uthash.h>
#include <lwip/udp.h>
#include "Net.h"
#include "utils.h"

typedef struct
{
    ServiceClient _clt;
}Client;


static ServiceClient* _clients = NULL;

static KThread _netServiceThread;
static Service _netService;
static char _netName[] = "NET";

int NetServiceInit()
{
    int error = 0;
    ServiceInit(&_netService, getKernelTaskProcess() );
    _netService.name = _netName;

    ServiceCreateKernelTask(&_netService);
    NameServerRegister(&_netService);
    return 0;
}

static void _Write(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    ServiceClient* clt = NULL;
    HASH_FIND_PTR(_clients, &caller, clt );
    assert(clt);

    KernelTaskContext* ctx = getKernelTaskContext();
    size_t sizeToWrite = seL4_GetMR(2);
    printf("Net Write ret for %zi bytes '%s'\n", sizeToWrite, clt->buff);

    NetSendTo(clt->buff, sizeToWrite);

    seL4_Reply(msg);
}

static void _Read(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    ServiceClient* clt = NULL;
    HASH_FIND_PTR(_clients, &caller, clt );
    assert(clt);

    KernelTaskContext* ctx = getKernelTaskContext();

    size_t sizeToRead = seL4_GetMR(2);

    NetBuffer* buff = getRXBuffer();
    KMutexLock(&buff->rxListMutex);
    if(LListSize(&buff->rxList))
    {
        printf("NetService: take an element\n");
        
        struct pbuf *buf = LListPut(&buff->rxList);
        printf("NetService Put a buff %p remains %i\n", buf, LListSize(&buff->rxList));

        size_t effectiveSize = NetSendPbuf(buf, clt->buff, sizeToRead);

        seL4_MessageInfo_t i = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
        seL4_SetMR(0, effectiveSize);
        KMutexUnlock(&buff->rxListMutex);    
        seL4_Reply(i);
        return;
    }
    KMutexUnlock(&buff->rxListMutex);

    caller->replyCap = get_free_slot(&ctx->vka);
    int error = cnode_savecaller(&ctx->vka, caller->replyCap);
    assert(error == 0);

    NetSetEndpoint(caller, clt->buff, seL4_GetMR(2));
}

static void _Bind(ThreadBase* caller, seL4_MessageInfo_t msg)
{
/*
    seL4_SetMR(1, familly);
    seL4_SetMR(2, protoc);
    seL4_SetMR(3, port);
*/
    KLOG_DEBUG("Net Bind message from %i\n", ProcessGetPID(caller->process));
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
    client->_clt.caller = caller;
    client->_clt.buff = buff;
    client->_clt.service = &_netService;
    HASH_ADD_PTR(_clients, caller,(ServiceClient*) client);
    seL4_SetMR(1, buffShared);
    seL4_Reply(msg);

    LL_APPEND(caller->clients, (ServiceClient*) client);
}

static void ClientCleanup(ServiceClient *clt)
{
    HASH_DEL(_clients, clt);
    Client* c = clt;


    free(c);
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

        if (caller->process == getKernelTaskProcess())
        {
            ServiceNotification notif = seL4_GetMR(0);
            if(notif == ServiceNotification_ClientExit)
            {
                ServiceClient* clt = seL4_GetMR(1);
                ClientCleanup(clt);
            }
        }
        else
        {        
            NetRequest req = (NetRequest) seL4_GetMR(0);
            switch (req)
            {
            case NetRequest_Register:
                _Register(caller, msg);
                break;
            case NetRequest_Bind:
                _Bind(caller, msg);
                break;
            case NetRequest_Read:
                _Read(caller, msg);
                break;
            case NetRequest_Write:
                _Write(caller, msg);
                break;
            default:
                KLOG_TRACE("[NetService] Received unknown code %i from %i\n", req, sender);
                assert(0);
                break;
            }
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