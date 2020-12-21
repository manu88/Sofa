#include "DKService.h"
#include "NameServer.h"
#include "Environ.h"
#include "KThread.h"
#include "Log.h"
#include "Process.h"
#include "DeviceKit/DeviceTree.h"
#include <dk.h>


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
    KernelTaskContext* env = getKernelTaskContext();
    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(_dkService.baseEndpoint, &sender);
        ThreadBase* caller =(ThreadBase*) sender;
        assert(caller->process);
        if (caller->process == getKernelTaskProcess())
        {
            KLOG_DEBUG("DKService: msg from kernel_task!\n");
        }
        else if(seL4_GetMR(0) == DKRequest_Register)
        {
            KLOG_DEBUG("DKService: Register request\n");
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
            ServiceClient* client = malloc(sizeof(ServiceClient));
            assert(client);
            memset(client, 0, sizeof(ServiceClient));
            client->caller = caller;
            client->buff = buff;
            client->service = &_dkService;
            //HASH_ADD_PTR(_clients, caller,(ServiceClient*) client);
            LL_APPEND(caller->clients, client);            
            seL4_SetMR(1, (seL4_Word) buffShared);
            seL4_Reply(msg);
        }
        else if(seL4_GetMR(0) == DKRequest_List)
        {
            KLOG_DEBUG("DKService: Dev Enum request\n");
            IODevice* dev = NULL;
            FOR_EACH_DEVICE(dev)
            {
                KLOG_INFO("'%s' type %i\n", dev->name, dev->type);
            }
        }
        else if(seL4_GetMR(0) == DKRequest_Tree)
        {
            KLOG_DEBUG("DKService: IONode tree request\n");
            DeviceTreePrint(DeviceTreeGetRoot());
        }

    }
    
}

int DKServiceStart()
{
    KThreadInit(&_dkThread);
    _dkThread.mainFunction = mainDKService;
    _dkThread.name = DeviceKitServiceName;
    int error = KThreadRun(&_dkThread, 254, NULL);

    return error;

}