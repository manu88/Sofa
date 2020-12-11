#include "VFSService.h"

#include <stdio.h>
#include <string.h>
#include <sel4/sel4.h>
#include <vka/capops.h>
#include "ext2.h"
#include "KThread.h"
#include "DeviceTree.h"
#include "NameServer.h"
#include "Environ.h"
#include "Process.h"
#include "utils.h"
#include <Sofa.h>
#include <utils/uthash.h>
#include "VFS.h"
#include "Log.h"
#include "ext2FS.h"


typedef struct
{
    File file;

    UT_hash_handle hh; /* makes this structure hashable */
    int index;

}FileHandle;

typedef struct
{
    ServiceClient _clt;

    FileHandle* files;
    int fileIndex;


}Client;

static KThread _vfsThread;
static Service _vfsService;
static char _vfsName[] = "VFS";

static ServiceClient* _clients = NULL;

int VFSServiceInit()
{
    KernelTaskContext* ctx = getKernelTaskContext();

    ServiceInit(&_vfsService, getKernelTaskProcess());
    _vfsService.name = _vfsName;
    ServiceCreateKernelTask(&_vfsService);

    NameServerRegister(&_vfsService);
    return 0;
}

static IODevice* _dev = NULL;

static int VFSServiceLs(Client* client, const char* path)
{
    VFS_File_Stat st;
    return VFSStat(path, &st);
}

static int VFSServiceOpen(Client* client, const char* path, int mode)
{
    File ff;
    int ret =  VFSOpen(path, mode,&ff);
    if(ret != 0)
    {
        return -ret;
    }
    FileHandle* f = malloc(sizeof(FileHandle));
    assert(f);
    f->file = ff;
    f->index = client->fileIndex++;
    HASH_ADD_INT(client->files, index, f);

    return f->index;
}

static int VFSServiceSeek(Client* client, int handle, size_t pos)
{
    FileHandle* file = NULL;
    HASH_FIND_INT(client->files, &handle, file);
    if(file == NULL)
    {
        return EINVAL;
    }

    return VFSSeek(file, pos);
}

static ssize_t VFSServiceWrite(Client* client, int handle, int size)
{
    FileHandle* file = NULL;
    HASH_FIND_INT(client->files, &handle, file);
    if(file == NULL)
    {
        return -EINVAL;
    }
    return VFSWrite(&file->file, client->_clt.buff, size);
}

static ssize_t VFSServiceRead(Client* client, int handle, int size)
{
    FileHandle* file = NULL;
    HASH_FIND_INT(client->files, &handle, file);
    if(file == NULL)
    {
        return -EINVAL;
    }

    return VFSRead(&file->file, client->_clt.buff, size);
}

static int VFSServiceClose(Client* client, int handle)
{
    FileHandle* file = NULL;
    HASH_FIND_INT(client->files, &handle, file);
    if(file == NULL)
    {
        return EINVAL;
    }
    HASH_DEL(client->files, file);
    int ret = VFSClose(&file->file);
    free(file);
    return ret;
}

static int _VFSCheckSuperBlock(IODevice* dev, VFSSupported* fsType)
{
    assert(fsType);

    if(ext2_probe(dev))
    {
        if(ext2_mount(dev, NULL))
        {
            printf("[VFSMount] ext2\n");
            getExt2FS()->data = dev;
            return VFSMount(getExt2FS(), "/ext");
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
    return 0;

}


static void ClientCleanup(ServiceClient *clt)
{
    HASH_DEL(_clients, clt);
    Client* c = clt;

    FileHandle* f = NULL;
    FileHandle* tmp = NULL;
    HASH_ITER(hh, c->files, f, tmp)
    {
        printf("close File handle %i (R=%zi/%zi)\n", f->index, f->file.readPos, f->file.size);
        VFSClose(&f->file);
        free(f);
    }

    free(c);
}

static int mainVFS(KThread* thread, void *arg)
{
    KernelTaskContext* env = getKernelTaskContext();

    KLOG_INFO("VFS Thread started\n");
    IODevice* dev = NULL;
    FOR_EACH_DEVICE(dev)
    {
        if(dev->type == IODevice_BlockDev)
        {
            int r = VFSAddDEvice(dev);
            printf("VFSAddDEvice '%s' retuned %i\n", dev->name, r);

        }
    }
    while (1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(_vfsService.baseEndpoint, &sender);

        ThreadBase* caller =(ThreadBase*) sender;
        assert(caller->process);
        // Specific message sent by kernel_task to notify the service about the client killed
        if (caller->process == getKernelTaskProcess())
        {
            ServiceNotification notif = seL4_GetMR(0);
            if(notif == ServiceNotification_ClientExit)
            {
                ServiceClient* clt = seL4_GetMR(1);
                ClientCleanup(clt);
            }
        }
        else if(seL4_GetMR(0) == VFSRequest_Register)
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
            assert(client);
            memset(client, 0, sizeof(Client));
            client->_clt.caller = caller;
            client->_clt.buff = buff;
            client->_clt.service = &_vfsService;
            HASH_ADD_PTR(_clients, caller,(ServiceClient*) client);
            seL4_SetMR(1, buffShared);
            seL4_Reply(msg);

            LL_APPEND(caller->clients, (ServiceClient*) client);
        }
        else
        {
            ServiceClient* _clt = NULL;
            HASH_FIND_PTR(_clients, &caller, _clt );
            assert(_clt);
            Client* clt = _clt;
            if(seL4_GetMR(0) == VFSRequest_ListDir)
            {
                const char* path = clt->_clt.buff;
                int ret = VFSServiceLs(clt, path);
                seL4_SetMR(1, ret);
                seL4_Reply(msg);
            }
            else if(seL4_GetMR(0) == VFSRequest_Open)
            {
                const char* path = clt->_clt.buff;
                int ret = VFSServiceOpen(clt, path, seL4_GetMR(1));
                int handle = ret>=0?ret:0;
                int err = ret <0? -ret:0;
                seL4_SetMR(1, err);
                seL4_SetMR(2, handle);            
                seL4_Reply(msg);

            }
            else if(seL4_GetMR(0) == VFSRequest_Close)
            {
                int handle = seL4_GetMR(1);
                int ret = VFSServiceClose(clt, handle);
                seL4_SetMR(1, ret);            
                seL4_Reply(msg);
            }
            else if(seL4_GetMR(0) == VFSRequest_Write)
            {
                int handle = seL4_GetMR(1);
                int size = seL4_GetMR(2);

                ssize_t ret = VFSServiceWrite(clt, handle, size);
                int err = ret<0? -ret:0;
                size = ret >=0? ret:0; 
                seL4_SetMR(1, err);
                seL4_SetMR(2, size);            
                seL4_Reply(msg);

            }
            else if(seL4_GetMR(0) == VFSRequest_Read)
            {
                int handle = seL4_GetMR(1);
                int size = seL4_GetMR(2);
                ssize_t ret = VFSServiceRead(clt, handle, size);
                int err = ret<0? -ret:0;
                size = ret >=0? ret:0; 
                seL4_SetMR(1, err);
                seL4_SetMR(2, size);            
                seL4_Reply(msg);
            }
            else if(seL4_GetMR(0) == VFSRequest_Seek)
            {
                int handle = seL4_GetMR(1);
                size_t offset = seL4_GetMR(2);
                int ret = VFSServiceSeek(clt, handle, offset);
                seL4_SetMR(1, ret);
                seL4_Reply(msg);
            }
            else if(seL4_GetMR(0) == VFSRequest_Debug)
            {
                printf("VFS Client Debug\n");

                printf("VFS has %i clients\n", HASH_COUNT(_clients));
                FileHandle* f = NULL;
                FileHandle* tmp = NULL;
                printf("Current index %i\n", clt->fileIndex);
                HASH_ITER(hh, clt->files, f, tmp)
                {
                    printf("File handle %i (R=%zi/%zi)\n", f->index, f->file.readPos, f->file.size);
                }
            }
            else
            {
                printf("Other VFS request %u\n", seL4_GetMR(0));
                seL4_Reply(msg);
            }
        }
    }
    
    return 42;
}

int VFSServiceStart()
{
    KLOG_INFO("--> Start VFSD thread\n");
    KThreadInit(&_vfsThread);
    _vfsThread.mainFunction = mainVFS;
    _vfsThread.name = "VFSD";
    int error = KThreadRun(&_vfsThread, 254, NULL);

    return error;
}