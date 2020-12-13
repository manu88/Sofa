#include "NetService.h"
#include "NameServer.h"
#include "KThread.h"
#include "Environ.h"
#include "Log.h"
#include "Process.h"
#include <lwip/udp.h>
#include <Sofa.h>
#include <utils/uthash.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lwip/udp.h>
#include "Net.h"
#include "utils.h"

typedef struct _MsgUDP
{
    struct _MsgUDP *next;

    struct pbuf *p;
    ip_addr_t addr;
    uint16_t port;
}MsgUDP;


typedef struct _Client Client;

typedef struct
{
    UT_hash_handle hh; /* makes this structure hashable */
    int index;

    int domain;
    int type;
    int protocol;

    struct udp_pcb *_udp;
    struct _Client* client;

    MsgUDP *rxList;
    size_t waitingSize;

}SocketHandle;


typedef struct _Client
{
    ServiceClient _clt;
    seL4_CPtr replyCap;

    SocketHandle* sockets;
    int sockIndex;

    KMutex rxBuffMutex;
    //size_t waitingSize;
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


static size_t NetSendPbuf(struct pbuf* p, void* cltBuf, size_t size);


static size_t CopyMemUDPToUser(const MsgUDP* msg, Client* client, size_t size)
{
    size_t addrSize = sizeof(struct sockaddr_in);
    struct sockaddr_in *addr =(struct sockaddr_in*)(client->_clt.buff);

    addr->sin_port = msg->port;

    char b[64];
    const char *strAddr = ipaddr_ntoa_r(&msg->addr, b, 64);
    assert(inet_pton(AF_INET, strAddr, &addr->sin_addr) == 1); 

    size_t ret = msg->p->tot_len;
    if(size < ret)
    {
        ret = size;
    }
    KLOG_DEBUG("CopyMemUDPToUser will copy %zi bytes\n", ret);
    char *dataPos = client->_clt.buff + addrSize;
    memcpy(dataPos, msg->p->payload, ret);
    return ret;
}
static ssize_t doRecvFrom(Client* client, SocketHandle* sock, size_t size ,size_t *addrSize)
{
    assert(sock);
    int storedMsgCount = -1;
    MsgUDP* el = NULL;
    
    KMutexLock(&sock->client->rxBuffMutex);
    LL_COUNT(sock->rxList, el, storedMsgCount);

    ssize_t ret = 0;
    if(storedMsgCount)
    {
        MsgUDP* pck = sock->rxList;
        LL_DELETE(sock->rxList, pck);
        *addrSize = sizeof(struct sockaddr_in);
        ret = CopyMemUDPToUser(pck, client, size);
        pbuf_free(pck->p);
        kfree(pck);

    }
    KMutexUnlock(&sock->client->rxBuffMutex);

    return ret;
}
static void _RecvFrom(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    ServiceClient* _clt = NULL;
    HASH_FIND_PTR(_clients, &caller, _clt );
    assert(_clt);
    Client* clt = (Client*)_clt;

    int handle = seL4_GetMR(1);
    SocketHandle* sock = NULL;
    HASH_FIND_INT(clt->sockets, &handle, sock);
    if(sock == NULL)
    {
        seL4_SetMR(2, -EINVAL);
        seL4_Reply(msg);
        return ;
    }

    size_t size = seL4_GetMR(2);
    size_t addrSize = 0;
    ssize_t ret = doRecvFrom(clt, sock, size, &addrSize);

    if(ret == 0)
    {
        printf("NetService Wait for more data\n");
        sock->waitingSize = size;

        KernelTaskContext* ctx = getKernelTaskContext();
        clt->replyCap = get_free_slot(&ctx->vka);
        int error = cnode_savecaller(&ctx->vka, clt->replyCap);
        return;
    }

    seL4_SetMR(1, ret);
    seL4_SetMR(2, addrSize);
    seL4_Reply(msg);



#if 0
    ServiceClient* _clt = NULL;
    HASH_FIND_PTR(_clients, &caller, _clt );
    assert(_clt);
    Client* clt = (Client*)_clt;

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
#endif
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
    /*
    This is called from an other IRQ thread, not in NetService's thread. Be careful.
    */
    SocketHandle* sock = arg;
    assert(sock);
    printf("_on_udp called\n");

    KMutexLock(&sock->client->rxBuffMutex);
    MsgUDP *msg = malloc(sizeof(MsgUDP));
    assert(msg);
    msg->p = p;
    msg->port = port;
    msg->addr = *addr;
    LL_APPEND(sock->rxList, msg);
    KMutexUnlock(&sock->client->rxBuffMutex);

    if(sock->waitingSize)
    {
        assert(sock->client->replyCap);
        KMutexLock(&sock->client->rxBuffMutex);
        MsgUDP* pck = sock->rxList;
        assert(pck);

        LL_DELETE(sock->rxList, pck);

        KMutexUnlock(&sock->client->rxBuffMutex);

        size_t addrSize = sizeof(struct sockaddr_in);
        size_t ret = CopyMemUDPToUser(pck, sock->client, sock->waitingSize);
        pbuf_free(pck->p);
        kfree(pck);

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0 ,0, 3);
        seL4_SetMR(1, ret);
        seL4_SetMR(2, addrSize);
        seL4_Send(sock->client->replyCap, info);
        
        KernelTaskContext* ctx = getKernelTaskContext();
        cnode_delete(&ctx->vka, sock->client->replyCap);
        sock->client->replyCap = 0;
        sock->waitingSize = 0;


    }
    return ;
#if 0
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
#endif
}

static int doUDPBind(Client* client, SocketHandle* sock, const struct sockaddr_in* sockAddr)
{
    int port = ntohs(sockAddr->sin_port);
    KLOG_TRACE("NetServiceBind bind socket to port %i and host '%s'\n",
            port,
            inet_ntoa(sockAddr->sin_addr)
            );

    if(sock->_udp)
    {
        KLOG_INFO("NetService.doUDPBind already bound\n");
        return -EISCONN;
    }

    KLOG_DEBUG("Create UDP thing\n");
    struct udp_pcb *udp = udp_new();
    assert(udp);
    udp_recv(udp, _on_udp, sock);

    ip_addr_t _addr;
    assert(ipaddr_aton(inet_ntoa(sockAddr->sin_addr), &_addr));
    err_t error_bind = udp_bind(udp, &_addr, port);
    if(error_bind != 0)
    {
        udp_remove(udp);
    }
    if(error_bind == ERR_USE)
    {
        return -EADDRINUSE;
    }
    else if(error_bind != 0)
    {
        KLOG_TRACE("Other lwip error %i to map to errno. Will assert\n", error_bind);
        assert(0);
        return -EINVAL;
    }

    sock->_udp = udp;

    return 0;
}


static int doBind(Client* client, int handle, const struct sockaddr *addr, socklen_t addrlen)
{
    SocketHandle* sock = NULL;
    HASH_FIND_INT(client->sockets, &handle, sock);
    if(sock == NULL)
    {
        KLOG_DEBUG("NetService.doBind socket handle %i not found\n", handle);
        return -EINVAL;
    }
    if(addrlen != sizeof(struct sockaddr_in))
    {
        KLOG_DEBUG("NetService.doBind size of addr do not match sizeof(struct sockaddr_in) (%i vs %zi)\n", addrlen, sizeof(struct sockaddr_in));
        return -EINVAL;
    }
    const struct sockaddr_in* sockAddr = (const struct sockaddr_in*) addr;


    KLOG_TRACE("domain=%i protoc=%i type=%i\n", sock->domain, sock->protocol, sock->type);

    if(sock->type == SOCK_DGRAM)
    {
        return doUDPBind(client, sock, sockAddr);
    }

    KLOG_INFO("NetService.doBind: unsupported protocol %i\n", sock->protocol);
    return -EPROTONOSUPPORT;
}

static void _Bind(ThreadBase* caller, seL4_MessageInfo_t msg)
{
/*
    (1, handle);
    (2, familly);
    (3, protoc);
    (4, port);
*/
    ServiceClient* _clt = NULL;
    HASH_FIND_PTR(_clients, &caller, _clt );
    assert(_clt);
    Client* clt = (Client*) _clt;
    int handle = seL4_GetMR(1);
    size_t addrLen = seL4_GetMR(2);
    const struct sockaddr *addr = (const struct sockaddr *) clt->_clt.buff;
    assert(addr);
    int ret = doBind(clt, handle, addr, addrLen);

    seL4_SetMR(1, ret);
    seL4_Reply(msg);
    return;

#if 0
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
#endif
}

static void _Socket(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    ServiceClient* _clt = NULL;
    HASH_FIND_PTR(_clients, &caller, _clt );
    assert(_clt);
    Client* clt = (Client*) _clt;


    SocketHandle* sock = malloc(sizeof(SocketHandle));
    assert(sock);
    memset(sock, 0, sizeof(SocketHandle));
    sock->client = clt;
    sock->index = clt->sockIndex++;
    HASH_ADD_INT(clt->sockets, index, sock);

    sock->domain = seL4_GetMR(1);//, domain);
    sock->type = seL4_GetMR(2);//, type);
    sock->protocol = seL4_GetMR(3);//, protocol);


    ssize_t ret = sock->index;
    printf("NetService.Socket ret %li\n", ret);
    int handle = ret>=0?ret:0;
    int err = ret <0? -ret:0;
    seL4_SetMR(1, err);
    seL4_SetMR(2, handle);            
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

    HASH_ADD_PTR(_clients, caller,(ServiceClient*) client);
    seL4_SetMR(1, (seL4_Word) buffShared);
    seL4_Reply(msg);

    LL_APPEND(caller->clients, (ServiceClient*) client);
}

static void ClientCleanup(ServiceClient *clt)
{
    HASH_DEL(_clients, clt);
    Client* c = (Client*)clt;


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
                ServiceClient* clt = (ServiceClient*) seL4_GetMR(1);
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
            case NetRequest_Socket:
                _Socket(caller, msg);
                break;
            case NetRequest_Bind:
                _Bind(caller, msg);
                break;
            case NetRequest_RecvFrom:
                _RecvFrom(caller, msg);
                break;
            case NetRequest_Write:
                _Write(caller, msg);
                break;
            default:
                KLOG_TRACE("[NetService] Received unknown code %i from %li\n", req, sender);
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