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

#include "Process.h"
#include "Bootstrap.h"
#include "Utils.h"
#include "Timer.h"
#include "NameServer.h"
#include "Config.h"

#include <vka/capops.h>

static Process* _procList = NULL;
//static list_t _processes = {0};
//#define MAX_PROCESSES 10
//static Process* procList[MAX_PROCESSES] = {0};

static uint32_t pidCounter = 1;

static Process *initProcess = NULL;

int ProcessListInit()
{
    return 0;
}

static void ProcRelease(KObject *p)
{
    free(p);
}

seL4_CPtr
sel4osapi_process_copy_cap_into(Process *process, vka_t *parent_vka, seL4_CPtr cap, seL4_CapRights_t rights)
{
    seL4_CPtr minted_cap;
    seL4_Word cap_badge = process->pid;
    cspacepath_t src_path;
    
    vka_cspace_make_path(parent_vka, cap, &src_path);
    minted_cap = sel4utils_mint_cap_to_process(&process->native, src_path, rights, cap_badge);
    assert(minted_cap != 0);
    
    return minted_cap;
}



int
sel4osapi_process_init_env(Process *process,
                           //int pid,
                           //char *name,
                           //uint8_t priority,
                           vka_t *parent_vka,
                           vspace_t *parent_vspace,
                           int user_untypeds_num,
                           uint8_t *user_untypeds_allocation,
                           uint8_t *user_untypeds_size_bits,
                           vka_object_t *user_untypeds
                           //seL4_CPtr sysclock_ep,
                           //seL4_CPtr udp_stack_ep
                           )
{
    int error;
    
    {
        int i = 0;
        uint32_t untyped_size = 0;
        uint32_t untyped_count = 0;
        
        //process->env->pid = pid;
        //snprintf(process->env->name, SEL4OSAPI_USER_PROCESS_NAME_MAX_LEN, "%s", name);
        
        //process->env->priority = priority;
        
        /* set up caps about the process */
        process->env->page_directory = sel4osapi_process_copy_cap_into(process, parent_vka, process->native.pd.cptr, seL4_AllRights);
        process->env->root_cnode = SEL4UTILS_CNODE_SLOT;
        process->env->tcb = sel4osapi_process_copy_cap_into(process, parent_vka, process->native.thread.tcb.cptr, seL4_AllRights);
        /* setup data about untypeds */
        for (i = 0; i < user_untypeds_num && untyped_size < SEL4OSAPI_USER_PROCESS_UNTYPED_MEM_SIZE; i++) {
            seL4_CPtr proc_ut_cap = 0;
            
            if (user_untypeds_allocation[i] != 0)
            {
                continue;
            }
            
            proc_ut_cap = sel4osapi_process_copy_cap_into(process, parent_vka, user_untypeds[i].cptr, seL4_AllRights);
            
            /* set up the cap range */
            if (untyped_count == 0) {
                process->env->untypeds.start = proc_ut_cap;
            }
            process->env->untypeds.end = proc_ut_cap;
            
            user_untypeds_allocation[i] = process->pid;
            
            process->env->untyped_size_bits_list[untyped_count] = user_untypeds_size_bits[i];
            process->env->untyped_indices[untyped_count] = i;
            untyped_count++;
            
            untyped_size += 1 << user_untypeds_size_bits[i];
        }
        assert((process->env->untypeds.end - process->env->untypeds.start) + 1 <= user_untypeds_num);
#ifdef CONFIG_LIB_OSAPI_SYSCLOCK
        {
            /* copy the sysclock's EP */
            process->env->sysclock_server_ep = sel4osapi_process_copy_cap_into(process, parent_vka, sysclock_ep, seL4_AllRights);
            assert(process->env->sysclock_server_ep != 0);
        }
#endif
        {
            /* allocate an AEP for the process' idling */
            vka_object_t aep_obj = { 0 };
            
            error = vka_alloc_notification(parent_vka, &aep_obj);
            assert(error == 0);
            
            //process->parent_idling_aep = aep_obj.cptr;
            
            //process->env->idling_aep = sel4osapi_process_copy_cap_into(process, parent_vka, process->parent_idling_aep, seL4_AllRights);
            
            assert(process->env->idling_aep != 0);
        }
#if 0
        {
            /* register process with ipc server */
            seL4_CPtr rx_pages[SEL4OSAPI_PROCESS_RX_BUF_PAGES];
            seL4_CPtr tx_pages[SEL4OSAPI_PROCESS_TX_BUF_PAGES];
            seL4_CPtr rx_pages_mint[SEL4OSAPI_PROCESS_RX_BUF_PAGES];
            seL4_CPtr tx_pages_mint[SEL4OSAPI_PROCESS_TX_BUF_PAGES];
            cspacepath_t dest, src;
            
            process->env->ipcclient.id = process->ipcclient->id;
            
            for (i = 0; i < SEL4OSAPI_PROCESS_RX_BUF_PAGES; ++i) {
                rx_pages[i] = vspace_get_cap(parent_vspace, process->ipcclient->rx_buf + i * PAGE_SIZE_4K);
                assert(rx_pages[i] != seL4_CapNull);
                vka_cspace_make_path(parent_vka, rx_pages[i], &src);
                error = vka_cspace_alloc(parent_vka, &rx_pages_mint[i]);
                assert(error == 0);
                vka_cspace_make_path(parent_vka, rx_pages_mint[i], &dest);
                error = vka_cnode_copy(&dest, &src, seL4_AllRights);
                assert(error == 0);
            }
            
            for (i = 0; i < SEL4OSAPI_PROCESS_TX_BUF_PAGES; ++i) {
                tx_pages[i] = vspace_get_cap(parent_vspace, process->ipcclient->tx_buf + i * PAGE_SIZE_4K);
                assert(tx_pages[i] != seL4_CapNull);
                vka_cspace_make_path(parent_vka, tx_pages[i], &src);
                error = vka_cspace_alloc(parent_vka, &tx_pages_mint[i]);
                assert(error == 0);
                vka_cspace_make_path(parent_vka, tx_pages_mint[i], &dest);
                error = vka_cnode_copy(&dest, &src, seL4_AllRights);
                assert(error == 0);
            }
            
            process->env->ipcclient.rx_buf = vspace_map_pages(&process->native.vspace, rx_pages_mint, NULL, seL4_AllRights, SEL4OSAPI_PROCESS_RX_BUF_PAGES, PAGE_BITS_4K, 1);
            assert(process->env->ipcclient.rx_buf != NULL);
            process->env->ipcclient.rx_buf_size = SEL4OSAPI_PROCESS_RX_BUF_SIZE;
            
            process->env->ipcclient.tx_buf = vspace_map_pages(&process->native.vspace, tx_pages_mint, NULL, seL4_AllRights, SEL4OSAPI_PROCESS_RX_BUF_PAGES, PAGE_BITS_4K, 1);
            assert(process->env->ipcclient.tx_buf != NULL);
            process->env->ipcclient.tx_buf_size = SEL4OSAPI_PROCESS_TX_BUF_SIZE;
        }
#endif
#ifdef CONFIG_LIB_OSAPI_NET
        {
            process->env->udp_iface.stack_op_ep = sel4osapi_process_copy_cap_into(process, parent_vka, udp_stack_ep, seL4_AllRights);
            assert(process->env->udp_iface.stack_op_ep != seL4_CapNull);
            
        }
#endif
#ifdef CONFIG_LIB_OSAPI_SERIAL
        {
            /* register process with network driver */
            seL4_CPtr buf_pages[SEL4OSAPI_SERIAL_BUF_PAGES];
            seL4_CPtr buf_pages_mint[SEL4OSAPI_SERIAL_BUF_PAGES];
            cspacepath_t dest, src;
            
            process->env->serial.id = process->serialclient->id;
            
            process->env->serial.server_ep = sel4osapi_process_copy_cap_into(process, parent_vka, process->serialclient->server_ep, seL4_AllRights);
            assert(process->env->serial.server_ep != seL4_CapNull);
            
            for (i = 0; i < SEL4OSAPI_SERIAL_BUF_PAGES; ++i) {
                buf_pages[i] = vspace_get_cap(parent_vspace, process->serialclient->buf + i * PAGE_SIZE_4K);
                assert(buf_pages[i] != seL4_CapNull);
                vka_cspace_make_path(parent_vka, buf_pages[i], &src);
                error = vka_cspace_alloc(parent_vka, &buf_pages_mint[i]);
                assert(error == 0);
                vka_cspace_make_path(parent_vka, buf_pages_mint[i], &dest);
                error = vka_cnode_copy(&dest, &src, seL4_AllRights);
                assert(error == 0);
            }
            
            process->env->serial.buf = vspace_map_pages(&process->native.vspace, buf_pages_mint, NULL, seL4_AllRights, SEL4OSAPI_SERIAL_BUF_PAGES, PAGE_BITS_4K, 1);
            assert(process->env->serial.buf != NULL);
            process->env->serial.buf_size = SEL4OSAPI_SERIAL_BUF_SIZE;
            
        }
#endif
        /* copy the fault endpoint - we wait on the endpoint for a message
         * or a fault to see when the test finishes */
        process->env->fault_endpoint = sel4osapi_process_copy_cap_into(process, parent_vka, process->native.fault_endpoint.cptr, seL4_AllRights);
        assert(process->env->fault_endpoint != 0);
        
        /* WARNING: DO NOT COPY MORE CAPS TO THE PROCESS BEYOND THIS POINT,
         * AS THE SLOTS WILL BE CONSIDERED FREE AND OVERRIDDEN BY THE TEST PROCESS. */
        /* set up free slot range */
        process->env->cspace_size_bits = CONFIG_SEL4UTILS_CSPACE_SIZE_BITS;
        process->env->free_slots.start = process->env->fault_endpoint + 1;
        process->env->free_slots.end = (1u << CONFIG_SEL4UTILS_CSPACE_SIZE_BITS);
        assert(process->env->free_slots.start < process->env->free_slots.end);
    }
    
    return 0;
}


void ProcessInit(Process*p)
{
    memset(p , 0 , sizeof(Process));
    
    KSetInit(&p->base);
    p->base.obj.methods.release = ProcRelease;
    assert(p->reply == 0);
    assert(p->replyState  == ReplyState_None);
}

Process* ProcessGetByPID( uint32_t pid)
{
    Process *p = NULL;
    
    HASH_FIND_INT( _procList, &pid, p );
    
    return p;
}

size_t ProcessGetChildrenCount(Process* process)
{
    return KSetCount( (const KSet*) process);
}

static int ProcessListAdd(Process* process)
{
    if( process->pid == 1) // we stach the init process
    {
        initProcess = process;
    }
    
    HASH_ADD_INT( _procList, pid, process );
    KObjectGet((KObject *)process);
    return 0;
}

static int ProcessListRemove(Process* process)
{
    HASH_DEL(_procList, process);
    KObjectPut((KObject *)process);
    return 0;
    
}


int ProcessSetPriority( Process* process , int prio)
{
#ifdef TEST_ONLY
    return -1;
#else
    int ret = seL4_TCB_SetPriority(process->native.thread.tcb.cptr , simple_get_tcb(getSimple()) , prio);
    if( ret == 0)
    {
        process->priority = prio;
    }
    return ret;
#endif
}
int ProcessGetPriority( Process* process , int *prio)
{
    *prio = process->priority;
    return 0;
}

int ProcessStart(Process *process , const char* procName, vka_object_t *fromEp , Process *parent)
{
    assert(process != parent);
    assert(strlen(procName) < MAX_PROCESS_NAME);
    
    assert(fromEp->cptr != 0);
   
    int error = 0;
    
    process->pid = pidCounter++;
    printf("[kernel_task] configure '%s' process -> pid %i\n" , procName , process->pid);
    assert(process->pid> 0);
    process->base.obj.k_name = strdup(procName);

#ifndef TEST_ONLY
    
    process->priority = SOFA_DEFAULT_PRIORITY;
    //sel4utils_process_config_t config = process_config_default_simple( getSimple(), procName, process->priority/*seL4_MaxPrio*/);
    
    sel4utils_process_config_t config = process_config_default(procName, seL4_CapInitThreadASIDPool);
    config = process_config_auth(config, simple_get_tcb(getSimple() ) );
    config = process_config_priority(config, process->priority);
    config = process_config_mcp(config, process->priority);
    
    
    config.create_fault_endpoint = false;
    /* badge the fault endpoint to use for messages such that we can distinguish them */
    cspacepath_t badged_ep_path;
    error = vka_cspace_alloc_path(getVka(), &badged_ep_path);
    if( error != 0)
    {
        printf( "Failed to allocate path");
    }
    cspacepath_t ep_path;
    vka_cspace_make_path(getVka(), fromEp->cptr, &ep_path);
    error = vka_cnode_mint(&badged_ep_path, &ep_path, seL4_AllRights, process->pid);
    
    if( error != 0)
    {
        printf( "Failed to badge ep");
    }
    
    
    config = process_config_fault_cptr(config ,badged_ep_path.capPtr );
    
    
    assert(config.create_fault_endpoint == false);
    error = sel4utils_configure_process_custom( &process->native , getVka() , getVspace(), config);
    if( error != 0)
    {
        printf("Error sel4utils_configure_process_custom %i\n" , error);
        return error;
    }

/*
    error = sel4osapi_process_init_env(process, getVka(), getVspace(),
                                       getSystem()->user_untypeds_num,
                                       getSystem()->user_untypeds_allocation,
                                       getSystem()->user_untypeds_size_bits,
                                       getSystem()->user_untypeds);
 
    assert(error == 0);
 */
/* END-fucking-POINTS Are HERE*/


    
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
        printf("Failed to sel4utils_mint_cap_to_process ep_cap_path\n");
        
        return -1;
    }
    
    
    
    
    char endpoint_string[10] = "";
    
    snprintf(endpoint_string, 10, "%ld", (long) process_ep_cap);
    
    // rule : the endpoint should be the last arg, and is gonna be removed from the arg list before main is called
    seL4_Word argc = 2;
    char *argv[] = {procName , endpoint_string};
    
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
    process->bufEnv = vspace_new_pages(getVspace(),seL4_AllRights , numPages , PAGE_BITS_4K);
    if( process->bufEnv == NULL)
    {
        printf("Error vspace_new_pages\n");
        return -1;
    }
    
    seL4_CPtr process_data_frame = vspace_get_cap(getVspace(), process->bufEnv);
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

#endif
    if( process->pid != 1) //
    {
        assert(parent);
        int ret = KSetAppend( (KSet*) parent , (KObject*) process);
        assert(ret == 0);
    }
    int r = ProcessListAdd(process);
    
    assert(r==0);
    return r;
}

size_t ProcessListGetCount()
 {
     return HASH_COUNT(_procList);
 }

static int ProcessSignalTerminaison( Process* process , Process* parent)
{
    if( parent->replyState == ReplyState_Wait)
    {
        printf("Parent %i %s is waiting for its child\n", parent->pid , ProcessGetName( parent));

        assert(parent->reply);
#ifndef TEST_ONLY
        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 4);
        seL4_SetMR(0, SysCall_Wait);
        seL4_SetMR(1, process->pid);
        seL4_SetMR(2, process->retCode);
        seL4_SetMR(3, process->retSignal);
        
        seL4_Send(parent->reply , tag);
        
        
        cnode_delete(getVka(),parent->reply);
#endif
        parent->reply = 0;

        parent->replyState = ReplyState_None;
        
        return 0;
    }
    
    return -1;
}

Process* ProcessGetFirstChild(Process* fromProcess)
{
    KObject* el = NULL;
    KSetForeach((KSet*) fromProcess, el)
    {
        Process* p = (Process*) el;
        return p;
    }
    return NULL;
}

Process* ProcessGetFirstChildZombie(Process* fromProcess)
{
    KObject* el = NULL;
    KSetForeach((KSet*) fromProcess, el)
    {
        Process* p = (Process*) el;
        if( p->status == ProcessState_Zombie)
        {
            return p;
        }
    }
    return NULL;
}



int ProcessCleanup( Process* process)
{
    KSetRemove((KSet*) ProcessGetParent(process) ,(KObject*) process);
    
    int error = ProcessListRemove(process);
    
    
    return error;
}
int ProcessKill( Process* process , SofaSignal signal)
{
    assert(initProcess);
    int error = -1;
    
    if( process->status == ProcessState_Zombie)
        return error;
    
    process->retSignal = signal;
    
    TimerCancelID(process->timerID); // cancel any pending timer
    
    assert(ProcessGetParent(process));
    

    KObject* el = NULL;
    KObject*tmp = NULL;

    KSetForeachSafe((KSet*)process,el,tmp)
    {
        Process* proc = (Process*) el;
        
        KSetAppend( (KSet*)initProcess ,(KObject*) proc);
        //proc->parent = initProcess;
    }

    int ret = ProcessSignalTerminaison(process , ProcessGetParent(process) );
    
    NameServerRemoveAllFromProcess(process);
    RemoveProcessAsClient(process);
#ifndef TEST_ONLY
    sel4utils_destroy_process(&process->native, getVka() );
#endif
    
    if( ret == 0)
    {
        if( process->reply)
        {
#ifndef TEST_ONLY
            cnode_delete(getVka(),process->reply);
#endif
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
    Process* proc = NULL;
    Process* temp = NULL;

    HASH_ITER(hh, _procList, proc, temp)
    {
        printf("%i '%s' status %i Parent %i (%li child)  replyState %i #syscalls %ld started at %li\n" ,
               proc->pid,
               ProcessGetName(proc) ,
               proc->status,
               ProcessGetParent(proc)?ProcessGetParent(proc)->pid:0 ,
               ProcessGetChildrenCount(proc),
               proc->replyState,
               proc->stats.numSysCalls,
               proc->stats.startedTime
               );
    }
}

    
void ProcessDumpCaps(Process *proc)
{
    printf("Process %i %s caps : \n" , proc->pid  ,ProcessGetName(proc) );
    
    if( proc->caps.caps == 0)
    {
        printf("No caps\n");
    }
    else
    {
        if (ProcessHasCap(proc , SofaCap_Nice))// (proc->caps.caps & SofaCap_Nice) == SofaCap_Nice)
        {
            printf("CapNice\n");
        }
        if (ProcessHasCap(proc , SofaCap_Kill))//((proc->caps.caps & SofaCap_Kill) == SofaCap_Kill)
        {
            printf("CapKill\n");
        }
        if (ProcessHasCap(proc , SofaCap_Spawn))//if ((proc->caps.caps & SofaCap_Spawn) == SofaCap_Spawn)
        {
            printf("CapSpawn\n");
        }
        if (ProcessHasCap(proc , SofaCap_CreateServer))//if ((proc->caps.caps & SofaCap_CreateServer) == SofaCap_CreateServer)
        {
            printf("CapCreateServer\n");
        }
        
    }
}
