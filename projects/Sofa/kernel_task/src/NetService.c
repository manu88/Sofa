/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "NetService.h"
#include "NameServer.h"
#include "Environ.h"
#include "Log.h"
#include "BaseService.h"
#include "ProcessList.h"

#include <ethdrivers/lwip.h>
#include <netif/etharp.h>
#include <lwip/udp.h>
#include <lwip/init.h>
#include <Sofa.h>
#include <utils/uthash.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lwip/udp.h>
#include "Net.h"
#include "utils.h"
#include "DeviceTree.h"
#include "IODevice.h"
#include "net.h"


static BaseService _service;
static char _netName[] = "NET";

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
}Client;

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg);
static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);
static void _OnServiceStart(BaseService* service);

static BaseServiceCallbacks _servOps = 
{
    .onServiceStart = _OnServiceStart,
    .onSystemMsg = _OnSystemMsg,
    .onClientMsg = _OnClientMsg
};

static int LWipErrToErrno(err_t err)
{
    switch (err)
    {
    case ERR_ARG:
        return -EINVAL;
    case ERR_RTE:
        return -EINVAL;
    default:
        KLOG_ERROR("LWipErrToErrno unhandled lwip err_t %i\n", err);
        assert(0);
        break;
    }
}

int NetServiceInit()
{
    lwip_init();
    udp_init();

    return BaseServiceCreate(&_service, _netName, &_servOps);
}

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
    Client* clt = (Client*) BaseServiceGetClient(&_service, caller);
    assert(clt);


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
        sock->waitingSize = size;

        KernelTaskContext* ctx = getKernelTaskContext();
        clt->replyCap = get_free_slot(&ctx->vka);
        int error = cnode_savecaller(&ctx->vka, clt->replyCap);
        return;
    }

    seL4_SetMR(1, ret);
    seL4_SetMR(2, addrSize);
    seL4_Reply(msg);
}

static void _on_udp(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    /*
    This is called from an other IRQ thread, not in NetService's thread. Be careful.
    */
    SocketHandle* sock = arg;
    assert(sock);

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
}

static int doUDPBind(Client* client, SocketHandle* sock, const struct sockaddr_in* sockAddr)
{
    int port = ntohs(sockAddr->sin_port);

    if(sock->_udp)
    {
        KLOG_INFO("NetService.doUDPBind already bound\n");
        return -EISCONN;
    }

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
        return -EINVAL;
    }
    if(addrlen != sizeof(struct sockaddr_in))
    {
        return -EINVAL;
    }
    const struct sockaddr_in* sockAddr = (const struct sockaddr_in*) addr;



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
    Client* clt = (Client*) BaseServiceGetClient(&_service, caller);
    assert(clt);
    int handle = seL4_GetMR(1);
    size_t addrLen = seL4_GetMR(2);
    const struct sockaddr *addr = (const struct sockaddr *) clt->_clt.buff;
    assert(addr);
    int ret = doBind(clt, handle, addr, addrLen);

    seL4_SetMR(1, ret);
    seL4_Reply(msg);
    return;

}

static int _EnumInterfaces(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("NetService enum interface\n");
    char b[64];
    struct netif* interface = NULL;
    NETIF_FOREACH(interface)
    {
        KLOG_DEBUG("%i: ", interface->num);    
        const char *strAddr = ipaddr_ntoa_r(&interface->ip_addr, b, 64);
        KLOG_DEBUG("inet %s ", strAddr);
        KLOG_DEBUG("mtu %u ", interface->mtu);
        KLOG_DEBUG("\n");
    }
    return 0;
}

static err_t _LoopbackNetifInit(struct netif *netif)
{
    return 0;
}

static int _createLoopbackInterface()
{
    ip_addr_t addr, mask;
    ipaddr_aton("127.0.0.1",   &addr);
    //ipaddr_aton("255.", &addr);
    ipaddr_aton("255.255.255.0", &mask);

    struct netif* netiface = malloc(sizeof(struct netif));
    if(!netiface)
    {
        return -ENOMEM;
    }

    if(netif_add(netiface, &addr, &mask, NULL,NULL, _LoopbackNetifInit, ethernet_input) != netiface)
    {
        KLOG_ERROR("NetService: Loopback interface creation error\n");
        return -1;
    }
    return 0;
}
static int _doAddIface(IODevice* dev, const char* addrStr, const char* gwStr, const char* maskStr)
{
    KLOG_DEBUG("_doAddIface '%s' gw '%s' mask '%s'\n", addrStr, gwStr, maskStr);

    ip_addr_t addr, gw, mask;
    ipaddr_aton(gwStr,   &gw);
    ipaddr_aton(addrStr, &addr);
    ipaddr_aton(maskStr, &mask);

    struct netif* netiface = malloc(sizeof(struct netif));
    if(!netiface)
    {
        return -ENOMEM;
    }

    int retRegister = dev->ops->regIface(dev, netiface, &addr, &gw, &mask);
    if(retRegister == 0)
    {
        netif_set_up(netiface);
        netif_set_default(netiface);
    }
    return retRegister;
}



static int _ConfigInterface(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    Client* clt = (Client*) BaseServiceGetClient(&_service, caller);
    assert(clt);

    void *devHandle = seL4_GetMR(1);
    KLOG_DEBUG("NetService Config interface request for device %p\n", devHandle);
    IODevice* dev = DeviceTreeGetDeviceFromHandle(devHandle);
    if(!dev)
    {
        return -1;
    }

    KLOG_DEBUG("NetService got device %p %s\n", dev, dev->name);
    const ConfigureInterfacePayload* payload = (const ConfigureInterfacePayload*) clt->_clt.buff;
    return _doAddIface(dev, payload->addr, payload->gw, payload->mask);
}

static int closeUdp(SocketHandle* sock)
{
    if(sock->_udp)
    {
        udp_remove(sock->_udp);
        sock->_udp = NULL;
    }
    MsgUDP* elt = NULL;
    MsgUDP* tmp = NULL;
    LL_FOREACH_SAFE(sock->rxList,elt,tmp)
    {
      LL_DELETE(sock->rxList, elt);
      pbuf_free(elt->p);
      free(elt);
    }
}

static ssize_t doClose(Client* client, int handle)
{
    SocketHandle* sock = NULL;
    HASH_FIND_INT(client->sockets, &handle, sock);
    if(sock == NULL)
    {
        return -EINVAL;
    }
    HASH_DELETE(hh, client->sockets, sock);

    ssize_t ret = closeUdp(sock);
    free(sock);
    return ret;

}

static void _Close(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    Client* clt = (Client*) BaseServiceGetClient(&_service, caller);
    assert(clt);

    int handle = seL4_GetMR(1);

    ssize_t ret = doClose(clt, handle);
    seL4_SetMR(1, ret);
    seL4_Reply(msg);
}

static void _Socket(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    Client* clt = (Client*) BaseServiceGetClient(&_service, caller);
    assert(clt);


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
    int handle = ret>=0?ret:0;
    int err = ret <0? -ret:0;
    seL4_SetMR(1, err);
    seL4_SetMR(2, handle);            
    seL4_Reply(msg);
}

static void _SendTo(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    Client* client = (Client*) BaseServiceGetClient(&_service, caller);
    assert(client);

    int handle = seL4_GetMR(1);
    SocketHandle* sock = NULL;
    HASH_FIND_INT(client->sockets, &handle, sock);
    if(sock == NULL)
    {
        seL4_SetMR(2, -EINVAL);
        seL4_SetMR(3, 0);
        seL4_Reply(msg);
        return;
    }

    size_t dataSize = seL4_GetMR(2);
    size_t addrSize = seL4_GetMR(3);

    //FIXME: check if udp
    assert(addrSize == sizeof(struct sockaddr_in));
    const struct sockaddr_in *addr = (const struct sockaddr_in *) client->_clt.buff;
    const char* dataPos = client->_clt.buff + addrSize;

    ip_addr_t dst_ip = {0};
    assert(ipaddr_aton(inet_ntoa(addr->sin_addr), &dst_ip));

    err_t err = ERR_MEM;
    assert(sock->_udp == NULL);
    sock->_udp = udp_new();

    size_t sentSize = 0;
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, dataSize, PBUF_RAM);
    if(p)
    {
        memcpy(p->payload, dataPos, dataSize);
        err = udp_sendto(sock->_udp, p, &dst_ip, addr->sin_port);
        pbuf_free(p);
        sentSize = dataSize;
    }
    else
    {
        assert(0);
    }

    //int sentLen = seL4_GetMR(2);
    //int err = seL4_GetMR(3);

    
    seL4_SetMR(2, LWipErrToErrno(err));
    seL4_SetMR(3, sentSize);
    seL4_Reply(msg);
}


static void _Register(ThreadBase* caller, seL4_MessageInfo_t msg)
{
    Client* client = malloc(sizeof(Client));
    if(!client)
    {
        seL4_SetMR(1, (seL4_Word) -1);
        seL4_Reply(msg);

        return;
    }
    memset(client, 0, sizeof(Client));
    int err = BaseServiceCreateClientContext(&_service, caller,(ServiceClient*) client, 1);
    if(err != 0)
    {
        free(client);
        seL4_SetMR(1, (seL4_Word) err);
        seL4_Reply(msg);
        return;
    }
    KMutexNew(&client->rxBuffMutex);
    
    seL4_SetMR(1, client? (seL4_Word)client->_clt.buffClientAddr: (seL4_Word) -1);        
    seL4_Reply(msg);

    
}

static void ClientCleanup(ServiceClient *clt)
{
    Client* c = (Client*)clt;

    SocketHandle* elt = NULL;
    SocketHandle* tmp = NULL;

    HASH_ITER(hh, c->sockets,elt, tmp )
    {
        HASH_DELETE(hh, c->sockets, elt);
        closeUdp(elt);
        free(elt);
    }

    free(c);

}

static void _OnServiceStart(BaseService* service)
{
    KLOG_INFO("NetService started\n");

    _createLoopbackInterface();
}

static void _OnClientMsg(BaseService* service, ThreadBase* caller, seL4_MessageInfo_t msg)
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
        case NetRequest_SendTo:
            _SendTo(caller, msg);
            break;
        case NetRequest_Close:
            _Close(caller, msg);
            break;
        case NetRequest_ConfigInterface:
        {
            int ret = _ConfigInterface(caller, msg);
            seL4_SetMR(1, ret);
            seL4_Reply(msg);
        }
            break;
        case NetRequest_EnumInterfaces:
        {
            int ret = _EnumInterfaces(caller, msg);
            seL4_SetMR(1, ret);
            seL4_Reply(msg);
        }
            break;
        default:
            KLOG_TRACE("[NetService] Received unknown code %i\n", req);
            assert(0);
            break;
        }
}

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("NetService _OnSystemMsg\n");
    ServiceNotification notif = seL4_GetMR(0);
    if(notif == ServiceNotification_ClientExit)
    {
        ServiceClient* clt = (ServiceClient*) seL4_GetMR(1);
        ClientCleanup(clt);
    }
    else if(notif == ServiceNotification_WillStop)
    {
        KLOG_DEBUG("[NetService] will stop\n");
    }
}


int NetServiceStart()
{
    return BaseServiceStart(&_service);
}