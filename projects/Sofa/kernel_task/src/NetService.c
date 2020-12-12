#include "NetService.h"
#include "NameServer.h"
#include "KThread.h"
#include "Environ.h"
#include "Log.h"
#include "Process.h"
#include <lwip/udp.h>
#include <Sofa.h>
#include <utils/uthash.h>
#include <netinet/in.h>
#include <lwip/udp.h>
#include "Net.h"
#include "utils.h"
#include "LList.h"


typedef struct
{
    char* buf;
    size_t writePos;
    size_t readPos;
    size_t size;
    size_t allocSize;

}MemBuffer;

void MemBufferInit(MemBuffer* b, size_t preallocSize)
{
    memset(b, 0, sizeof(MemBuffer));

    b->buf = malloc(preallocSize);
    b->allocSize = preallocSize;
}

int MemBufferPushBack(MemBuffer*b, char* data ,size_t dataSize)
{
    if(b->writePos + dataSize > b->allocSize)
    {
        b->buf = realloc(b->buf, b->writePos + dataSize);
    }
    strncpy(b->buf + b->writePos, data, dataSize);
    b->writePos += dataSize;

    b->size += dataSize;
    return 0;
}


typedef struct
{
    ServiceClient _clt;
    seL4_CPtr replyCap;
    LList _udps;

    MemBuffer rxBuf;

    KMutex rxBuffMutex;
    size_t waitingSize;
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

    assert(0);
    seL4_Reply(msg);
}

static size_t ClientGetReadBufferLenBytes(Client* clt)
{
    return clt->rxBuf.size;
}

static size_t NetSendPbuf(struct pbuf* p, void* cltBuf, size_t size);

static void _Read(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    ServiceClient* _clt = NULL;
    HASH_FIND_PTR(_clients, &caller, _clt );
    assert(_clt);
    Client* clt = _clt;

    KernelTaskContext* ctx = getKernelTaskContext();


    size_t sizeToRead = seL4_GetMR(2);

    KMutexLock(&clt->rxBuffMutex);

    size_t sizeAvail = ClientGetReadBufferLenBytes(clt);
    printf("NetService read req %i bytes, got %zi avail\n", sizeToRead, sizeAvail);

    if(sizeAvail < sizeToRead)
    {
        printf("NetService Wait for more data\n");
        clt->waitingSize = sizeToRead;

        clt->replyCap = get_free_slot(&ctx->vka);
        int error = cnode_savecaller(&ctx->vka, clt->replyCap);
        assert(error == 0);
    }
    else
    {
        memcpy(clt->_clt.buff, clt->rxBuf.buf + clt->rxBuf.readPos, sizeToRead);
        clt->rxBuf.readPos += sizeToRead;
        clt->rxBuf.size -= sizeToRead;

        size_t effectiveSize = sizeToRead;

        seL4_SetMR(0, effectiveSize);
        seL4_Reply(msg);

    }
    KMutexUnlock(&clt->rxBuffMutex);
}

static size_t ClientAppendReadBuffer(Client* clt, struct pbuf* p)
{
    KMutexLock(&clt->rxBuffMutex);

    MemBufferPushBack(&clt->rxBuf, p->payload, p->len);

    KMutexUnlock(&clt->rxBuffMutex);

    return clt->rxBuf.size;    
}

static size_t NetSendPbuf(struct pbuf* p, void* cltBuf, size_t size)
{
    size_t effectiveSize = size;
    if(p->tot_len < effectiveSize)
    {
        effectiveSize = p->tot_len;
    }

    void* ptr = pbuf_get_contiguous(p, cltBuf, 4096, effectiveSize, 0); 
    if(ptr== NULL)
    {
        effectiveSize = 0;
    }
    if(ptr != cltBuf)
    {
        memcpy(cltBuf, ptr, effectiveSize);
    }

    return effectiveSize;
}

static void _on_udp(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    Client* clt = arg;
    assert(clt);
    assert(LListSize(&clt->_udps));
    printf("_on_udp called\n");

    size_t bufSize = ClientAppendReadBuffer(clt, p);
    pbuf_free(p);
    
    KLOG_DEBUG("NetService.OnUDP %zi\n", bufSize);

    if (clt->replyCap && bufSize >= clt->waitingSize)
    {
        printf("Client is waiting for %zi data to read\n", clt->waitingSize);
        KMutexLock(&clt->rxBuffMutex);

        memcpy(clt->_clt.buff, clt->rxBuf.buf + clt->rxBuf.readPos, clt->waitingSize);
        clt->rxBuf.readPos += clt->waitingSize;
        clt->rxBuf.size -= clt->waitingSize;
        KMutexUnlock(&clt->rxBuffMutex);

        size_t effectiveSize = clt->waitingSize; //NetSendPbuf(buf, clt->_clt.buff, clt->waitingSize);
        //pbuf_free(buf);

        seL4_CPtr ep = clt->replyCap;
        assert(ep);
        clt->replyCap = 0;
        clt->waitingSize = 0;

        seL4_MessageInfo_t i = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
        seL4_SetMR(0, effectiveSize);
        seL4_Send(ep, i);
    }
}

static void _Bind(ThreadBase* caller, seL4_MessageInfo_t msg)
{
/*
    seL4_SetMR(1, familly);
    seL4_SetMR(2, protoc);
    seL4_SetMR(3, port);
*/
    ServiceClient* _clt = NULL;
    HASH_FIND_PTR(_clients, &caller, _clt );
    assert(_clt);
    Client* clt = _clt;

    KLOG_DEBUG("Net Bind message from %i\n", ProcessGetPID(caller->process));
    int familly = seL4_GetMR(1);
    int protoc = seL4_GetMR(2);
    int port = seL4_GetMR(3);
    KLOG_INFO("fam=%i protoc=%i port=%i\n", familly, protoc, port);

    if (protoc == SOCK_DGRAM)
    {
        KLOG_DEBUG("Create UDP thing\n");
        struct udp_pcb *udp = udp_new();
        assert(udp);
        udp_recv(udp, _on_udp, clt);
        err_t error_bind = udp_bind(udp, NULL, port);
        if(error_bind == ERR_USE)
        {
            udp_remove(udp);
            seL4_Reply(msg);
            return;
        }
        else if(error_bind != 0)
        {
            KLOG_DEBUG("bind err %i\n", error_bind);
            assert(error_bind == ERR_OK);
        }

        LListAppend(&clt->_udps, udp);
    }

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
    KMutexNew(&client->rxBuffMutex);

    MemBufferInit(&client->rxBuf, 64);
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