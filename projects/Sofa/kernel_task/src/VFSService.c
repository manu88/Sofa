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
#include "VFSService.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sel4/sel4.h>
#include <vka/capops.h>
//#include "ext2.h"
#include "Ext2.h"
#include "KThread.h"
#include "DeviceTree.h"
#include "NameServer.h"
#include "BaseService.h"
#include "Environ.h"
#include "ProcessList.h"
#include "utils.h"
#include <Sofa.h>
#include <utils/uthash.h>
#include "VFS.h"
#include "MBR.h"
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

    char* workingDir;

}Client;



static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg);
static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);
static void _OnVFSStart(BaseService* service);

static BaseServiceCallbacks _servOps = 
{
    .onServiceStart = _OnVFSStart,
    .onSystemMsg = _OnSystemMsg,
    .onClientMsg = _OnClientMsg
};

static BaseService _vfsService;
static char _vfsName[] = "VFS";

int VFSServiceInit()
{
    int r = BaseServiceCreate(&_vfsService, _vfsName, &_servOps);
    if(r==0)
    {
        ServiceSetFlag(&_vfsService.service, ServiceFlag_Clone);
    }
    return r;
}


int PathIsAbsolute(const char* path)
{
    return path[0] == '/';
}


char* ConcPath(const char* workingDir, const char* path)
{
    size_t wdSize = strlen(workingDir);
    const size_t totalSize = wdSize + strlen(path);

    char* realP = malloc(totalSize+1);
    memset(realP, 0, totalSize+1);
    if(!realP)
    {
        return NULL;
    }
    if( wdSize > 1 && workingDir[wdSize-1] == '/' && path[0] == '/') 
    {
        wdSize -=1;
    }
    strcpy(realP, workingDir);
    strcpy(realP + wdSize, path);

    return realP;
}
static int VFSServiceChDIR(Client* clt)
{
    const char* path = clt->_clt.buff;

    char *realP = NULL;
    int freeRealP = 0;
    int absolute = PathIsAbsolute(path);
    if(absolute)
    {
        realP = path;
    }
    else if (strlen(path) == 0)
    {
        realP = clt->workingDir;
    }
    else
    {
        realP = ConcPath(clt->workingDir, path);
        if(!realP)
        {
            return -ENOMEM;
        }
        freeRealP = 1;
    }
    
    VFS_File_Stat f;
    int err = VFSStat(realP, &f);
    if(err == 0 && f.type == FileType_Dir)
    {
        free(clt->workingDir);
        size_t pSize = strlen(realP);
        int addLastSlash = 0;
        if(realP[pSize-1] != '/')
        {
            addLastSlash =1;
        }
        clt->workingDir = malloc(pSize + 1 + addLastSlash);
        assert(clt->workingDir);
        strcpy(clt->workingDir, realP);
        if(addLastSlash)
        {
            clt->workingDir[pSize] = '/';    
        }

        clt->workingDir[pSize + addLastSlash] = 0;
        
    }
    else if(err == 0)
    {
        err = -EISDIR;
    }


    if(freeRealP)
    {
        free(realP);
    }

    return err;

}

static int modifyBit(int n, int p, int b) 
{ 
    int mask = 1 << p; 
    return (n & ~mask) | ((b << p) & mask); 
} 

static int _doVFSStatToStructStat( const VFS_File_Stat* statIn, struct stat* statOut)
{
    int mode = 0;
    switch (statIn->type)
    {
    case FileType_Regular:
        mode = S_IFREG;
        break;
    case FileType_Dir:
        mode = S_IFDIR;
        break;
    case FileType_Block:
        mode = S_IFBLK;
        break;
    case FileType_Char:
        mode = S_IFCHR;
        break;
    default:
        KLOG_ERROR("_doVFSStatToStructStat: unhandled file type %i\n", statIn->type);
        assert(0);
        break;
    }

    statOut->st_mode = mode;// | S_IFMT;
    return 0;
}

static int VFSServiceStat(Client* client, const char* path)
{
    char *realP = NULL;
    int freeRealP = 0;
    int absolute = PathIsAbsolute(path);
    if(absolute)
    {
        realP = path;
    }
    else if (strlen(path) == 0)
    {
        realP = client->workingDir;
    }
    else
    {
        realP = ConcPath(client->workingDir, path);
        if(!realP)
        {
            return -ENOMEM;
        }
        freeRealP = 1;
    }

    VFS_File_Stat st;
    int ret = VFSStat(realP, &st);

    if(freeRealP)
    {
       free(realP);
    }
    if(ret != 0)
    {
        return ret;
    }
    
    ret = _doVFSStatToStructStat(&st, (struct stat*) client->_clt.buff);

    return ret;
}
static int VFSServiceOpen(Client* client, const char* path, int mode)
{
    char *realP = NULL;
    int freeRealP = 0;
    int absolute = PathIsAbsolute(path);
    if(absolute)
    {
        realP = path;
    }
    else if (strlen(path) == 0)
    {
        realP = client->workingDir;
    }
    else
    {
        realP = ConcPath(client->workingDir, path);
        if(!realP)
        {
            return -ENOMEM;
        }
        freeRealP = 1;
    }

    File ff;
    int ret =  VFSOpen(realP, mode,&ff);
    if(ret != 0)
    {
        if(freeRealP)
        {
            free(realP);
        }
        return ret;
    }
    FileHandle* f = malloc(sizeof(FileHandle));
    assert(f);
    f->file = ff;
    f->index = client->fileIndex++;
    HASH_ADD_INT(client->files, index, f);
    if(freeRealP)
    {
        free(realP);
    }

    return f->index;
}

static int VFSServiceSeek(Client* client, int handle, off_t pos, int whence)
{
    FileHandle* file = NULL;
    HASH_FIND_INT(client->files, &handle, file);
    if(file == NULL)
    {
        return -EINVAL;
    }

    return VFSSeek(&file->file, pos, whence);
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

static ssize_t VFSServiceRead(Client* client, int handle, int size, int* asyncResult)
{
    FileHandle* file = NULL;
    HASH_FIND_INT(client->files, &handle, file);
    if(file == NULL)
    {
        return -EINVAL;
    }
    return VFSRead(client->_clt.caller, &file->file, client->_clt.buff, size, asyncResult);
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

    if(HASH_COUNT(client->files) == 0)
    {
        KLOG_DEBUG("Reset fd counter \n");
        client->fileIndex = 0;
    }
    return ret;
}


static int testPartition(IODevice* dev, const PartitionTableEntry* ent)
{
    if(ent->active == 0 && 
       ent->systemID == 0
    )
    {
        KLOG_DEBUG("\t-->Empty partition table\n");
        return -1;
    }
    else
    {
        KLOG_DEBUG("\t-->active %X systemID %X num sectors %d lba start %d start sector %X\n", ent->active, ent->systemID, ent->numSectors, ent->lbaStart, ent->startSector);
        
        if(Ext2Probe(dev, ent->lbaStart))
        {
            if(Ext2Mount(dev))
            {
                KLOG_DEBUG("[VFSMount] ext2\n");
                getExt2FS()->data = dev;
                int err = 0;
                VFSMount(getExt2FS(), "/ext", &err);
                return err;
            }
            KLOG_DEBUG("ext2_mount error for '%s'\n", dev->name);
        }
        else
        {
            KLOG_DEBUG("ext2_probe error for '%s'\n", dev->name);
            return -1;
        }
    }

}

static int _VFSInspectDisk(IODevice* dev, VFSSupported* fsType)
{
    assert(fsType);

    uint8_t data[512] = {0};
    ssize_t ret = IODeviceRead(dev, 0, data, 512);
    if(ret == 512)
    {
        const MBR* mbr =(const MBR*) data;

        KLOG_DEBUG("MBR diskID=%X validboot=%X\n", mbr->diskID, mbr->validBoot);
        // validBoot should be == 0XAA55 and diskID != 0
        if(mbr->diskID != 0 && mbr->validBoot == 0XAA55)
        {
            KLOG_DEBUG("Found a valid MBR, check partitions\n");
            KLOG_DEBUG("Check partition1\n");
            testPartition(dev, &mbr->part1);
            KLOG_DEBUG("Check partition2\n");
            testPartition(dev, &mbr->part2);
            KLOG_DEBUG("Check partition3\n");
            testPartition(dev, &mbr->part3);
            KLOG_DEBUG("Check partition4\n");
            testPartition(dev, &mbr->part4);
        }
        else // single partition disk
        {
            KLOG_DEBUG("No MBR, mount disk directly\n");
            if(Ext2Probe(dev, 0))
            {
                if(Ext2Mount(dev))
                {
                    KLOG_DEBUG("[VFSMount] ext2\n");
                    getExt2FS()->data = dev;
                    int err = 0;
                    VFSMount(getExt2FS(), "/ext", &err);
                    return err;
                }
                KLOG_DEBUG("ext2_mount error for '%s'\n", dev->name);
            }
            else
            {
                KLOG_DEBUG("ext2_probe error for '%s'\n", dev->name);
            }
        }
    }
    return -1;


    if(Ext2Probe(dev, 0))
    {
        if(Ext2Mount(dev))
        {
            KLOG_DEBUG("[VFSMount] ext2\n");
            getExt2FS()->data = dev;
            int err = 0;
            VFSMount(getExt2FS(), "/ext", &err);
            return err;
        }
        KLOG_DEBUG("ext2_mount error for '%s'\n", dev->name);
    }
    else
    {
        KLOG_DEBUG("ext2_probe error for '%s'\n", dev->name);
    }

    return -1;
}


int VFSAddDEvice(IODevice *dev)
{
    KLOG_DEBUG("[VFS] add device '%s'\n", dev->name);

    VFSSupported type = VFSSupported_Unknown;
    int test = _VFSInspectDisk(dev, &type);
    if(test != 0)
    {
        return test;
    }
    KLOG_DEBUG("[VFS] device  '%s' added with FS type %i\n", dev->name, type);
    return 0;

}

int VFSServiceGetClientCWD(ThreadBase* sender, char** pat_ret)
{
    Client* client = (Client*) BaseServiceGetClient(&_vfsService, sender);
    if(client == NULL)
    {
        return -ESRCH;
    }
    if(client->workingDir == NULL)
    {
        return -EINVAL;
    }
    *pat_ret = client->workingDir;
    return 0;
}

static Client* RegisterClient(ThreadBase* caller)
{
    Client* client = malloc(sizeof(Client));
    if(!client)
    {
        return NULL;
    }
    memset(client, 0, sizeof(Client));
    int err = BaseServiceCreateClientContext(&_vfsService, caller,(ServiceClient*) client, 1);
    if(err == 0)
    {
        client->workingDir = strdup("/");
    }
    else
    {
        free(client);
        return NULL;
    }
    return client;
}

static void ClientClone(ThreadBase* parent, ThreadBase* newProc)
{
    assert(parent->process);
    assert(newProc->process);
    if(newProc->process->state != ProcessState_Running)
    {
        return;
    }
    if(parent->process->state != ProcessState_Running)
    {
        return;
    }

    
    Client* newClient = RegisterClient(newProc);
    assert(newClient);

    Client* parentClient = (Client*) BaseServiceGetClient(&_vfsService, parent);


    FileHandle* f = NULL;
    FileHandle* tmp = NULL;
    HASH_ITER(hh, parentClient->files, f, tmp)
    {
        FileHandle* hdl = malloc(sizeof(FileHandle));
        memset(hdl, 0, sizeof(FileHandle));
        if(hdl)
        {
            hdl->file = f->file;
            hdl->index = f->index;
            HASH_ADD_INT(newClient->files, index, hdl);
        }
    }
    newClient->fileIndex = parentClient->fileIndex;
    newClient->workingDir = strdup(parentClient->workingDir);
}

static void ClientCleanup(ServiceClient *clt)
{
    Client* c = (Client*) clt;

    FileHandle* f = NULL;
    FileHandle* tmp = NULL;
    HASH_ITER(hh, c->files, f, tmp)
    {
        VFSClose(&f->file);
        free(f);
    }
    free(c->workingDir);
    free(c);
}

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    ServiceNotification notif = seL4_GetMR(0);
    if(notif == ServiceNotification_ClientExit)
    {
        ServiceClient* clt = (ServiceClient*) seL4_GetMR(1);

        ClientCleanup(clt);
    }
    else if(notif == ServiceNotification_WillStop)
    {
        KLOG_DEBUG("[VFSService] will stop\n");
    }
    else if(notif == ServiceNotification_Clone)
    {
        ThreadBase* parent =(ThreadBase*) seL4_GetMR(1);
        ThreadBase* newProc =(ThreadBase*) seL4_GetMR(2);

        ClientClone(parent, newProc);

        seL4_Reply(msg);
    }
}

static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    VFSRequest req = (VFSRequest) seL4_GetMR(0); 

    switch (req)
    {
    case VFSRequest_Register:
    {
        int err = 0;
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        if(client == NULL)
        {
            client = RegisterClient(sender);
        }
        seL4_SetMR(1, client? (seL4_Word)client->_clt.buffClientAddr: (seL4_Word) -1);        
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_Open:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);
        const char* path = client->_clt.buff;
        int ret = VFSServiceOpen(client, path, seL4_GetMR(1));
        int handle = ret>=0?ret:0;
        int err = ret <0? -ret:0;
        seL4_SetMR(1, err);
        seL4_SetMR(2, handle);            
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_Close:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);
        int handle = seL4_GetMR(1);
        int ret = VFSServiceClose(client, handle);
        seL4_SetMR(1, ret);            
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_Read:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);
        int handle = seL4_GetMR(1);
        int size = seL4_GetMR(2);

        int asyncLater = -1;
        ssize_t ret = VFSServiceRead(client, handle, size, &asyncLater);
        if(asyncLater == 1)
        {
            // VFS Backend will handle the reply to this msg later
            return;
        }
        int err = ret<0? ret:0;
        size = ret >=0? ret:0; 
        seL4_SetMR(1, err);
        seL4_SetMR(2, size);            
        seL4_Reply(msg);
    }

        break;
    case VFSRequest_Write:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        if(!client)
        {
            KLOG_ERROR("Client not found for sender %i\n", ProcessGetPID(sender->process));
        }
        assert(client);

        int handle = seL4_GetMR(1);
        int size = seL4_GetMR(2);

        ssize_t ret = VFSServiceWrite(client, handle, size);
        int err = ret<0? -ret:0;
        size = ret >=0? ret:0; 
        seL4_SetMR(1, err);
        seL4_SetMR(2, size);            
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_Seek:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);
        int handle = seL4_GetMR(1);
        off_t offset = seL4_GetMR(2);
        int whence = seL4_GetMR(3);
        int ret = VFSServiceSeek(client, handle, offset, whence);
        seL4_SetMR(1, ret);
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_GetCWD:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);

        size_t maxSize = seL4_GetMR(1);
        size_t pathSize = strlen(client->workingDir);
        if(pathSize < maxSize)
        {
            maxSize = pathSize;
        }
        strncpy(client->_clt.buff, client->workingDir, maxSize);
        client->_clt.buff[maxSize] = 0;
        seL4_SetMR(1, maxSize);
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_ChDir:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);
        int r = VFSServiceChDIR(client);
        seL4_SetMR(1, r);
        seL4_Reply(msg);
    }
        break;
    case VFSRequest_Stat:
    {
        Client* client = (Client*) BaseServiceGetClient(service, sender);
        assert(client);
        const char* path = client->_clt.buff;
        int err = VFSServiceStat(client, path);    
        seL4_SetMR(1, err);
        seL4_Reply(msg);         
    }
        break;
    
    default:
    assert(0);
        break;
    }
}

static void _OnVFSStart(BaseService* service)
{
    KLOG_INFO("VFSService started\n");
    IODevice* dev = NULL;
    FOR_EACH_DEVICE(dev)
    {
        if(dev->type == IODevice_BlockDev)
        {
            int r = VFSAddDEvice(dev);
        }
    }
}

int VFSServiceStart()
{
    return BaseServiceStart(&_vfsService);
}