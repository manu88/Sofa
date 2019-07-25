/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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
#include <utils/list.h>
#include "Process.h"
#include "Bootstrap.h"
#include "Utils.h"
#include "Timer.h"
#include "NameServer.h"

static list_t _processes = {0};
//#define MAX_PROCESSES 10
//static Process* procList[MAX_PROCESSES] = {0};

static uint32_t pidCounter = 1;

static Process *initProcess = NULL;

int ProcessListInit()
{
    int ret = -1;
    
    ret = list_init(&_processes);
    if( ret != 0)
    {
        return ret;
    }
    
    return ret;
}

void ProcessInit(Process*p)
{
    memset(p , 0 , sizeof(Process));
    
    assert(p->reply == 0);
    assert(p->replyState  == ReplyState_None);
}

Process* ProcessGetByPID( uint32_t pid)
{
    
    for (struct list_node *n = _processes.head; n != NULL; n = n->next)
    {
        Process* p = n->data;
        if( p->pid == pid)
            return p;

    }

    return NULL;
}

int ProcessGetChildrenCount(Process* process)
{
    int a = 0;
    
    for (struct list_node *n = _processes.head; n != NULL; n = n->next)
    {
        Process* p = n->data;
        if( p->parent == process)
        {
            a++;
        }
    }

    return a;
}

static int ProcessAdd(Process* process)
{
    if( process->pid == 1) // we stach the init process
    {
        initProcess = process;
    }
    return list_prepend(&_processes , process);
    /*
    for(int i=0;i<MAX_PROCESSES;i++)
    {
        if( procList[i] == NULL)
        {
            procList[i] = process;
            return 0;
        }
    }
    return -1;
     */
}
static int ProcComp(void*a, void*b)
{
    Process* p1 = a;
    Process* p2 = b;
    
    if( p1->pid == p2->pid)
    {
        return 0; // if equals
    }
    return -1;
}

static int ProcessRemove(Process* process)
{
    return list_remove(&_processes , process ,ProcComp );
    
}


int ProcessStart(Process *process , const char* procName, vka_object_t *fromEp , Process *parent)
{
    assert(strlen(procName) < MAX_PROCESS_NAME);
    
    assert(fromEp->cptr != 0);
   
    int error = 0;
    
    printf("[kernel_task] configure '%s' process\n" , procName);
    
    process->pid = pidCounter++;
    assert(process->pid> 0);
    process->parent = parent;
    sel4utils_process_config_t config = process_config_default_simple( getSimple(), procName, seL4_MaxPrio);
    
    error = sel4utils_configure_process_custom( &process->native , getVka() , getVspace(), config);
    if( error != 0)
    {
        printf("Error sel4utils_configure_process_custom %i\n" , error);
        return error;
    }
    
    strcpy(process->name , procName);
    /* END-fucking-POINTS Are HERE*/
    
    /* create a FAULT endpoint */
    
    /* allocate a cspace slot for the fault endpoint */
    
    seL4_CPtr fault_ep = 0;
    error = vka_cspace_alloc(
                             getVka(),
                             &fault_ep);
    
    ZF_LOGF_IFERR(error , "Failed to allocate thread fault endpoint");
    assert(error == 0);
    /* create a badged fault endpoint for the thread */
    error = seL4_CNode_Mint(
                            simple_get_cnode( getSimple()),
                            fault_ep,
                            seL4_WordBits,
                            seL4_CapInitThreadCNode,
                            fromEp->cptr,
                            seL4_WordBits,
                            seL4_AllRights,
                            process->pid
                            );
    ZF_LOGF_IF(error != 0, "Failed to mint badged fault endpoint for thread");
    assert(error == 0);
    
    config = process_config_fault_cptr(config, fault_ep);
    
    /**/
    
    /**/
    // This needs to be done AFTER sel4utils_configure_process_custom so that process->_process is valid
    seL4_CPtr process_ep_cap = 0;
    /// make a cspacepath for the new endpoint cap
    cspacepath_t ep_cap_path;
    
    
    vka_cspace_make_path( getVka(), fromEp->cptr, &ep_cap_path);
    
    process_ep_cap = sel4utils_mint_cap_to_process(&process->native,
                                                   ep_cap_path,
                                                   seL4_AllRights,
                                                   process->pid);
    
    if (process_ep_cap == 0)
    {
        printf("Failed to mint a badged copy of the IPC endpoint into the new thread's CSpace.\n"
               "\tsel4utils_mint_cap_to_process takes a cspacepath_t: double check what you passed.\n");
        
        return -1;
    }
    
    
    /* ---- */
    
    char endpoint_string[10] = "";
    
    snprintf(endpoint_string, 10, "%ld", (long) process_ep_cap);
    
    // rule : the endpoint should be the last arg, and is gonna be removed from the arg list before main is called
    seL4_Word argc = 2;
    char *argv[] = {process->name , endpoint_string};
    
    printf("[kernel_task] Start process \n");
    
    process->stats.startedTime = GetCurrentTime();
    error = sel4utils_spawn_process_v(&process->native , getVka() , getVspace() , argc, argv , 1);
    
    if( error != 0)
    {
        printf("Error sel4utils_spawn_process_v %i\n" , error);
        return error;
    }
    
    /* Shared mem */
    size_t numPages = 1;
    process->env = vspace_new_pages(getVspace(),seL4_AllRights , numPages , PAGE_BITS_4K);
    if( process->env == NULL)
    {
        printf("Error vspace_new_pages\n");
        return -1;
    }
    
    seL4_CPtr process_data_frame = vspace_get_cap(getVspace(), process->env);
    if( process_data_frame == 0)
    {
        printf("vspace_get_cap failed\n");
        return -1;
    }
    
    seL4_CPtr process_data_frame_copy = 0;
    sel4osapi_util_copy_cap(getVka(), process_data_frame, &process_data_frame_copy);
    if( process_data_frame_copy == 0)
    {
        printf("sel4osapi_util_copy_cap failed\n");
        return -1;
    }
    
    
    process->vaddr = vspace_map_pages(&process->native.vspace, &process_data_frame_copy, NULL, seL4_AllRights, numPages, PAGE_BITS_4K, 1/*cacheable*/);
    if( process->vaddr == NULL)
    {
        printf("vspace_map_pages error \n");
        return -1;
    }
    assert(process->vaddr != 0);
    /*END Shared mem*/

    return ProcessAdd(process);
}


static int ProcessSignalTerminaison( Process* process , Process* parent)
{
    if( parent->replyState == ReplyState_Wait)
    {
        printf("Parent %i %s is waiting for its child\n", parent->pid , parent->name);
        
        assert(parent->reply);
        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
        seL4_SetMR(0, SysCall_Wait);
        seL4_SetMR(1, process->pid);
        seL4_SetMR(2, process->retCode);
        
        seL4_Send(parent->reply , tag);
        
        
        cnode_delete(getVka(),parent->reply);
        parent->reply = 0;
        parent->replyState = ReplyState_None;
        
        return 0;
    }
    
    return -1;
}

Process* ProcessGetFirstChild(Process* fromProcess)
{
    for (struct list_node *n = _processes.head; n != NULL; n = n->next)
    {
        Process* proc = n->data;
        if( proc->parent == fromProcess)
        {
            return proc;
        }
    }
    return NULL;
}

int ProcessCleanup( Process* process)
{
    int error = ProcessRemove(process);
    
    free(process);
    return error;
}
int ProcessKill( Process* process)
{
    assert(initProcess);
    int error = -1;
    

    TimerCancelID(process->timerID); // cancel any pending timer
    
    assert(process->parent);
    
    if( ProcessGetChildrenCount(process))
    {
   
        for (struct list_node *n = _processes.head; n != NULL; n = n->next)
        {
            Process* proc = n->data;
            if( proc->parent == process)
            {
                proc->parent = initProcess;
            }
        }
    }
    int ret = ProcessSignalTerminaison(process , process->parent);
    
    NameServerRemoveAllFromProcess(process);
    RemoveProcessAsClient(process);
    
    sel4utils_destroy_process(&process->native, getVka() );
    
    if( ret == 0)
    {
        if( process->reply)
        {
            cnode_delete(getVka(),process->reply);
            process->reply = 0;
        }
        
        ProcessCleanup(process);
        
    }
    else
    {
        error = 0;
        process->status = ProcessState_Zombie;
    }
    
    return error;
}

void ProcessDump()
{
    for (struct list_node *n = _processes.head; n != NULL; n = n->next)
    {
        Process* proc = n->data;
        
        printf("%i '%s' status %i Parent %i TimerID %x reply %p replyState %i #syscalls %ld started at %li\n" ,
               proc->pid,
               proc->name ,
               proc->status,
               proc->parent?proc->parent->pid:0 ,
               proc->timerID,
               (void*)proc->reply,
               proc->replyState,
               proc->stats.numSysCalls,
               proc->stats.startedTime
               );
    }
    
}

    
