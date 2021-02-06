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
#include "ProcService.h"
#include "BaseService.h"
#include "Process.h"
#include "Log.h"
#include <proc.h>
#include <VFSService.h> // for PathIsAbsolute and ConcPath

static BaseService _service;

int doKill(pid_t pidToKill, ThreadBase* sender, int signal);
long doSpawn(ThreadBase* caller, const char* dataBuf);
long doWait(Process* process, pid_t pidToWait, int options, int *retCode);

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg);
static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg);
static BaseServiceCallbacks _servOps = {.onServiceStart=NULL, .onSystemMsg=_OnSystemMsg, .onClientMsg=_OnClientMsg};


int ProcServiceInit()
{
    return BaseServiceCreate(&_service, procServiceName, &_servOps);
}

int ProcServiceStart()
{
    return BaseServiceStart(&_service);
}

static void _OnSystemMsg(BaseService* service, seL4_MessageInfo_t msg)
{
    KLOG_DEBUG("ProcService.on system msg\n");
}


static void onRegister(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    ServiceClient* client = malloc(sizeof(ServiceClient));
    assert(client);
    int err = BaseServiceCreateClientContext(service, sender, client, 1);
    if(err != 0)
    {
        free(client);
    }
    seL4_SetMR(1, err == 0? client->buffClientAddr:-1);
    seL4_Reply(msg);
}


static void onProcEnum(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{

    ServiceClient* client = BaseServiceGetClient(service, sender);
    assert(client);

    size_t procCount = ProcessListCount();
    //KLOG_DEBUG("Proc enum request: %zi processes\n", procCount);    

    Process*p = NULL;

    size_t accSize = 0;
    char* buff = client->buff;

    ProcessListLock();
    FOR_EACH_PROCESS(p)
    {

        ProcessDesc* desc = (ProcessDesc*) buff;
        desc->startTime = p->stats.startTime;
        desc->pid = ProcessGetPID(p);
        desc->nameLen = strlen(ProcessGetName(p));
        desc->state = p->state;
        strncpy(desc->name, ProcessGetName(p), desc->nameLen);
        size_t recSize = sizeof(ProcessDesc) + desc->nameLen;

        accSize += recSize; 

        buff += recSize;
    }
    ProcessListUnlock();
    seL4_SetMR(1, procCount);
    seL4_Reply(msg);
}

static void onProcKill(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    pid_t pidToKill = seL4_GetMR(1);
    int signal = seL4_GetMR(2);

    int ret = doKill(pidToKill, sender, signal);

    if(pidToKill == ProcessGetPID(sender->process))
    {
        // dont bother send a reply, we're dead!
        return;
    }

    seL4_SetMR(1, ret);
    seL4_Reply(msg);
}

static void onProcWait(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    pid_t pidToWait = seL4_GetMR(1);
    int options = seL4_GetMR(2);
    int retCode = 0;
    long r = doWait(sender->process, pidToWait, options, &retCode);

    if(r == -EWOULDBLOCK)
    {
        KernelTaskContext* ctx = getKernelTaskContext();
        vka_t *mainVKA = getMainVKA();
        seL4_Word slot = get_free_slot(mainVKA);
        int error = cnode_savecaller(mainVKA, slot);
        if (error)
        {
            KLOG_TRACE("[Syscall_wait] Unable to save caller err=%i\n", error);
            cnode_delete(mainVKA, slot);
            seL4_SetMR(1, -error);
            seL4_Reply(msg);
            return;
        }
        
        sender->replyCap = slot;
        sender->state = ThreadState_Waiting;
    }
    else
    {
        seL4_SetMR(1, r);
        if(r>0)
        {
            seL4_SetMR(2, retCode); // wstatus
        }
        seL4_Reply(msg);
    }
}


static long onProcSpawn(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{

    ServiceClient* client = BaseServiceGetClient(service, sender);
    assert(client);

    long ret = 0;
    if(PathIsAbsolute(client->buff))
    {
        ret = doSpawn(sender, client->buff);
        return ret;
    }

    char* cltPath = NULL;
    ret = VFSServiceGetClientCWD(sender, &cltPath);
    if(ret != 0)
    {
        KLOG_DEBUG("onProcSpawn: unable to get working dir\n");
        return ret;
    }
    KLOG_DEBUG("onProcSpawn: CWD is '%s'\n", cltPath);

    char* realP = ConcPath(cltPath, client->buff);
    if(!realP)
    {
        return -ENOMEM;
    }

    ret = doSpawn(sender, realP);
    free(realP);
    return ret;
}


static void _OnClientMsg(BaseService* service, ThreadBase* sender, seL4_MessageInfo_t msg)
{
    ProcRequest req = (ProcRequest) seL4_GetMR(0);

    switch (req)
    {
    case ProcRequest_Register:
        onRegister(service, sender, msg);
        break;
    case ProcRequest_Enum:
        onProcEnum(service, sender, msg);
        break;
    case ProcRequest_Kill:
        onProcKill(service, sender, msg);
        break;
    case ProcRequest_Spawn:
    {
        long ret = onProcSpawn(service, sender, msg);
        seL4_SetMR(1, ret);
        seL4_Reply(msg);

        break;
    }
    case ProcRequest_Wait:
        onProcWait(service, sender, msg);
        break;
    default:
        break;
    } 
}