#include "VFSService.h"

#include <stdio.h>
#include <string.h>
#include <sel4/sel4.h>

#include "ext2.h"
#include "KThread.h"
#include "DeviceTree.h"
#include "NameServer.h"
#include "Environ.h"
#include "Process.h"
#include <Sofa.h>
#include <utils/uthash.h>

typedef struct
{
    ThreadBase* caller;
    UT_hash_handle hh; /* makes this structure hashable */
    char* buff;
}Client;

static KThread _vfsThread;
static Service _vfsService;
static char _vfsName[] = "VFS";

static Client* _clients = NULL;

int VFSInit()
{
    
    int error = 0;
    ServiceInit(&_vfsService, getKernelTaskProcess() );
    _vfsService.name = _vfsName;

    vka_object_t ep = {0};
    error = vka_alloc_endpoint(&getKernelTaskContext()->vka, &ep);
    assert(error == 0);
    _vfsService.baseEndpoint = ep.cptr;
    NameServerRegister(&_vfsService);
    return 0;
}

static IODevice* _dev = NULL;
void VFSLs(const char* path)
{
    printf("ls request '%s'\n", path);
    inode_t ino;
    ext2_find_file_inode(path, &ino, _dev, NULL);
    //ext2_list_directory(path, &ino, _dev, NULL);
    return;
    //uint8_t ret = ext2_read_root_directory(path, _dev, NULL);
    ext2_dir dir;
    uint32_t ret =  ext2_read_directory(path, &dir, _dev, NULL);
    printf("Ret %u\n", ret);
}

static int _VFSCheckSuperBlock(IODevice* dev, VFSSupported* fsType)
{
    assert(fsType);

    if(ext2_probe(dev))
    {
        if(ext2_mount(dev, NULL))
        {
            return 0;
        }
        printf("ext2_mount error for '%s'\n", dev->name);
    }
    else
    {
        printf("ext2_probe error for '%s'\n", dev->name);
    }

    return -1;
}


int VFSAddDEvice(IODevice *dev)
{
    printf("[VFS] add device '%s'\n", dev->name);
    _dev = dev;

    VFSSupported type = VFSSupported_Unknown;
    int test = _VFSCheckSuperBlock(dev, &type);
    if(test != 0)
    {
        return test;
    }
    printf("[VFS] device  '%s' added with FS type %i\n", dev->name, type);
}

static int mainVFS(KThread* thread, void *arg)
{
    KernelTaskContext* env = getKernelTaskContext();
    printf("Test Thread started\n");
    IODevice* dev = NULL;
    FOR_EACH_DEVICE(dev)
    {
        if(dev->type == IODevice_BlockDev)
        {
            VFSAddDEvice(dev);
        }
    }

    while (1)
    {
        printf("VFSD wait on msg\n");
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(_vfsService.baseEndpoint, &sender);
        ThreadBase* caller =(ThreadBase*) sender;
        assert(caller->process); 
        printf("VFSD got message from %s %i\n", ProcessGetName(caller->process), ProcessGetPID(caller->process));

        if(seL4_GetMR(0) == VFSRequest_Register)
        {
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
            client->caller = caller;
            client->buff = buff;
            HASH_ADD_PTR(_clients, caller, client);
            seL4_SetMR(1, buffShared);
            seL4_Reply(msg);
        }
        else if(seL4_GetMR(0) == VFSRequest_ListDir)
        {
            Client* clt = NULL;
            HASH_FIND_PTR(_clients, &caller, clt );
            assert(clt);
            const char* path = clt->buff;
            printf("List request for path '%s'\n", path);
            VFSLs(path);
            seL4_Reply(msg);
        }
        else
        {
            printf("Other VFS request %u\n", seL4_GetMR(0));

            Client* clt = NULL;
            HASH_FIND_PTR(_clients, caller, clt );
            assert(clt);
            seL4_Reply(msg);
        }
        

    }
    
    return 42;
}

int VFSStart()
{
    printf("--> Start VFSD thread\n");
    KThreadInit(&_vfsThread);
    _vfsThread.mainFunction = mainVFS;
    _vfsThread.name = "VFSD";
    int error = KThreadRun(&_vfsThread, 254, NULL);
    if( error != 0)
    {
        return error;
    }
    assert(error == 0);
    printf("<-- Start VFSD thread\n");

    return 0;
}