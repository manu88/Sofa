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
/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

/* Include Kconfig variables. */
#include <autoconf.h>

#include <sel4debug/register_dump.h>
#include <sel4utils/vspace_internal.h>
#include <vka/capops.h>
#include <Sofa.h>
#include "Environ.h"
#include "Process.h"
#include "utils.h"
#include "Panic.h"
#include "Log.h"
#include "NameServer.h"
#include "Timer.h"


extern Process initProcess;


int ProcessCreateSofaIPCBuffer(Process* p, void** addr, void** procAddr)
{
    KernelTaskContext* ctx = getKernelTaskContext();
    vspace_t* mainVSpace = getMainVSpace();
    *addr = (uint8_t*) vspace_new_pages(mainVSpace, seL4_AllRights, 1, PAGE_BITS_4K);
    assert(*addr);
    *procAddr = vspace_share_mem(mainVSpace, &p->native.vspace, *addr, 1, PAGE_BITS_4K, seL4_ReadWrite, 1);
    assert(*procAddr);

    return 0;
}

static int ProcessCloneServices(Process* parent, Process* p)
{
    assert(parent);
    ServiceClient* clt = NULL;
    ServiceClient* tmp = NULL;

    Thread* t = &parent->main;
    LL_FOREACH_SAFE(t->_base._clients, clt, tmp)
    {
        assert(clt->service);
        assert(clt->service->name);
        if(ServiceHasFlag(clt->service, ServiceFlag_Clone))
        {
            seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
            seL4_SetMR(0, ServiceNotification_Clone);
            seL4_SetMR(1, (seL4_Word) t); //
            seL4_SetMR(2, (seL4_Word) &p->main);
            seL4_Send(clt->service->kernTaskEp, info);
        }
    }

    return 0;
}

void spawnApp(Process* p, const char* imgName, Process* parent)
{
    KernelTaskContext* envir = getKernelTaskContext();
    static int pidPool = 1;
    vspace_t* mainVSpace = getMainVSpace();
    p->init = (test_init_data_t *) vspace_new_pages(mainVSpace, seL4_AllRights, 1, PAGE_BITS_4K);
    assert(p->init != NULL);
    p->init->pid = pidPool++;
    p->init->priority = seL4_MaxPrio - 1;

    int consumed_untypeds = process_set_up(NULL, p, imgName,(seL4_Word) &p->main);

    ProcessCreateSofaIPCBuffer(p,(void**) &p->main.ipcBuffer, (void**) &p->init->mainIPCBuffer);

    if(parent != NULL)
    {
        ProcessAddChild(parent, p);
        assert(p != &initProcess); // we only allow NULL parent for the first process
    }
    ProcessListAdd(p);
    if(parent)
    {
        ProcessCloneServices(parent, p);
    }
    else
    {
        assert(p == &initProcess);
    }
    
    process_run(imgName, p);
}

static void replyToWaitingParent(Thread* onThread, int pid, int retCode)
{
    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3);
    seL4_SetMR(0, 4);
    seL4_SetMR(1, pid);
    seL4_SetMR(2, retCode);
    seL4_Send(onThread->_base.replyCap, tag);

    cnode_delete(getMainVKA(), onThread->_base.replyCap);
    onThread->_base.replyCap = 0;
}


void _closeThreadClients(Thread*t)
{
    ServiceClient* clt = NULL;
    ServiceClient* tmp = NULL;

    LL_FOREACH_SAFE(t->_base._clients, clt, tmp)
    {
        LL_DELETE(t->_base._clients, clt);
        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, ServiceNotification_ClientExit);
        seL4_SetMR(1, (seL4_Word) clt);
        seL4_Send(clt->service->kernTaskEp, info);
    }
}

void doExit(Process* process, int retCode)
{
    KLOG_DEBUG("doExit for process %i\n", ProcessGetPID(process));
    if(ProcessGetPID(process) == 1)
    {
        Panic("init returned");
    }
// Close all client that belong to the process
    process->state = ProcessState_Stopped;
    _closeThreadClients(&process->main);
    Thread* thread = NULL;
    PROCESS_FOR_EACH_EXTRA_THREAD(process, thread)
    {
        _closeThreadClients(thread);
        KLOG_DEBUG("doExit.Suspend thread\n");
        seL4_TCB_Suspend(thread->tcb.cptr);
        KLOG_DEBUG("doExit.Suspend thread ok\n");

        KLOG_DEBUG("start vspace_free_ipc_buffer\n");
        vspace_free_ipc_buffer(&thread->_base.process->native.vspace, (seL4_Word *) thread->ipcBuf2);
        KLOG_DEBUG("end vspace_free_ipc_buffer\n");

        KLOG_DEBUG("start vspace_free_sized_stack\n");
        vspace_free_sized_stack(&thread->_base.process->native.vspace, thread->stack, thread->stackSize);
        KLOG_DEBUG("end vspace_free_sized_stack\n");

    }

//reap the children to init
    Process* parent = process->parent;
    // Do we have soon-to-be orphean children?
    if(ProcessCoundChildren(process))
    {
        Process* c = NULL;
        PROCESS_FOR_EACH_CHILD(process, c)
        {
            ProcessAddChild(&initProcess, c);
        }
    }

// Is our parent waiting on us?
    Thread* waitingThread = ProcessGetWaitingThread(parent);
    uint8_t freeProcess = 0;
    if(waitingThread)
    {
        if(waitingThread->_base.replyCap)
        {
            replyToWaitingParent(waitingThread, ProcessGetPID(process), retCode);
        }
        else
        {
            KLOG_TRACE("%i is waiting on %i, but no reply cap present\n", ProcessGetPID(parent), ProcessGetPID(process));
        }
        
//        assert(waitingThread->_base.replyCap != 0);
        freeProcess = 1;

        ProcessRemoveChild(parent, process);
        ProcessListRemove(process);        
    }
    else // Zombie time
    {
        process->retCode = retCode;
        process->state = ProcessState_Zombie;
    }

    int pidToFree = process->init->pid;

// any Services owned?
    Service* s = NULL;
    Service* t = NULL;
    FOR_EACH_SERVICE(s, t)
    {
        if(s->owner == process)
        {
            NameServerUnregister(s);
            assert(s->endpoint != seL4_CapNull);
            kfree(s->name);
            kfree(s);
        }
    }

//remove all except data needeed for the waiting parent
    process_tear_down(process);

// If nobody is waiting on us, release 
    if(freeProcess)
    {
        kfree(process);
    }
}

int process_set_up(uint8_t* untyped_size_bits_list, Process* process,const char* imgName, seL4_Word badge)
{
    KernelTaskContext* env = getKernelTaskContext();
    vka_t *mainVKA = getMainVKA();
    int error;

    //root_task_endpoint

    sel4utils_process_config_t config = process_config_default_simple(&env->simple, imgName, process->init->priority);
    config = process_config_mcp(config, seL4_MaxPrio);
    config = process_config_auth(config, simple_get_tcb(&env->simple));
    config = process_config_create_cnode(config, TEST_PROCESS_CSPACE_SIZE_BITS);
// create a badged faut ep
    cspacepath_t badged_ep_path;
    error = vka_cspace_alloc_path(mainVKA, &badged_ep_path);
    ZF_LOGF_IFERR(error, "Failed to allocate path\n");
    assert(error == 0);
    cspacepath_t ep_path = {0};
    vka_cspace_make_path(mainVKA, env->root_task_endpoint.cptr, &ep_path);

    error = vka_cnode_mint(&badged_ep_path, &ep_path, seL4_AllRights, badge);
    assert(error == 0);

    config = process_config_fault_cptr(config, badged_ep_path.capPtr);
//    
    vspace_t* mainVSpace = getMainVSpace();
    error = sel4utils_configure_process_custom(&process->native, mainVKA, mainVSpace, config);
    if(error != 0)
    {
        return error;
    }

    /* set up caps about the process */
    process->init->stack_pages = CONFIG_SEL4UTILS_STACK_SIZE / PAGE_SIZE_4K;
    process->init->stack = process->native.thread.stack_top - CONFIG_SEL4UTILS_STACK_SIZE;
    process->init->page_directory = sel4utils_copy_cap_to_process(&process->native, mainVKA, process->native.pd.cptr);
    process->init->root_cnode = SEL4UTILS_CNODE_SLOT;
    process->init->tcb = sel4utils_copy_cap_to_process(&process->native, mainVKA, process->native.thread.tcb.cptr);
    if (config_set(CONFIG_HAVE_TIMER)) {
        process->init->timer_ntfn = sel4utils_copy_cap_to_process(&process->native, mainVKA, env->timer_notify_test.cptr);
    }

    process->init->domain = sel4utils_copy_cap_to_process(&process->native, mainVKA, simple_get_init_cap(&env->simple,
                                                                                                           seL4_CapDomain));
    process->init->asid_pool = sel4utils_copy_cap_to_process(&process->native, mainVKA, simple_get_init_cap(&env->simple,
                                                                                                              seL4_CapInitThreadASIDPool));
    process->init->asid_ctrl = sel4utils_copy_cap_to_process(&process->native, mainVKA, simple_get_init_cap(&env->simple,
                                                                                                              seL4_CapASIDControl));
#ifdef CONFIG_IOMMU
    env->init->io_space = sel4utils_copy_cap_to_process(&process->native, &env->vka, simple_get_init_cap(&env->simple,
                                                                                                             seL4_CapIOSpace));
#endif /* CONFIG_IOMMU */
#ifdef CONFIG_TK1_SMMU
    env->init->io_space_caps = arch_copy_iospace_caps_to_process(&process->native, &env);
#endif
    process->init->cores = simple_get_core_count(&env->simple);
    /* copy the sched ctrl caps to the remote process */
    if (config_set(CONFIG_KERNEL_MCS)) {
        seL4_CPtr sched_ctrl = simple_get_sched_ctrl(&env->simple, 0);
        process->init->sched_ctrl = sel4utils_copy_cap_to_process(&process->native, mainVKA, sched_ctrl);
        for (int i = 1; i < process->init->cores; i++) {
            sched_ctrl = simple_get_sched_ctrl(&env->simple, i);
            sel4utils_copy_cap_to_process(&process->native, mainVKA, sched_ctrl);
        }
    }

/* setup data about untypeds */
    int num_untyped_per_process = 0;

    process->init->vspace_root = sel4utils_copy_cap_to_process(&process->native, mainVKA, vspace_get_root(&process->native.vspace));

    // create a minted enpoint for the process
    cspacepath_t path;
    vka_cspace_make_path(mainVKA, env->root_task_endpoint.cptr/* test_process.fault_endpoint.cptr*/, &path);
    
    process->main.process_endpoint = sel4utils_mint_cap_to_process(&process->native, path, seL4_AllRights, badge );


    /* copy the device frame, if any */
    if (process->init->device_frame_cap) {
        process->init->device_frame_cap = sel4utils_copy_cap_to_process(&process->native, mainVKA, env->device_obj.cptr);
    }

    /* map the cap into remote vspace */

    process->init_remote_vaddr = vspace_share_mem(mainVSpace, &process->native.vspace, process->init, 1, PAGE_BITS_4K,
                                         seL4_CanRead, 1);

    assert(process->init_remote_vaddr != 0);

    /* WARNING: DO NOT COPY MORE CAPS TO THE PROCESS BEYOND THIS POINT,
     * AS THE SLOTS WILL BE CONSIDERED FREE AND OVERRIDDEN BY THE TEST PROCESS. */
    /* set up free slot range */
    process->init->cspace_size_bits = TEST_PROCESS_CSPACE_SIZE_BITS;
    if (process->init->device_frame_cap) {
        process->init->free_slots.start = process->init->device_frame_cap + 1;
    } else {
        process->init->free_slots.start = process->main.process_endpoint + 1;
    }
    process->init->free_slots.end = (1u << TEST_PROCESS_CSPACE_SIZE_BITS);
    assert(process->init->free_slots.start < process->init->free_slots.end);

    return num_untyped_per_process;
}


void process_run(const char *name, Process* process)
{
    KernelTaskContext* env = getKernelTaskContext();
    int error;

    strncpy(process->init->name, name, TEST_NAME_MAX);

    process->init->name[TEST_NAME_MAX - 1] = '\0';
    process->name = process->init->name;

#ifdef CONFIG_DEBUG_BUILD
    seL4_DebugNameThread(process->native.thread.tcb.cptr, ProcessGetName(process));// process->init->name);
#else
#error "CONFIG_DEBUG_BUILD not set!"
#endif

    /* set up args for the test process */
    seL4_Word argc = 2 + process->argc;
    char string_args[argc][WORD_STRING_SIZE];
    char *argv[argc];

    argv[0] = string_args[0];
    argv[1] = string_args[1];    
    snprintf(argv[0], WORD_STRING_SIZE, "%"PRIuPTR"", (long unsigned int) process->main.process_endpoint);
    snprintf(argv[1], WORD_STRING_SIZE, "%"PRIuPTR"", (long unsigned int) process->init_remote_vaddr);
    for(int i=0;i<process->argc;i++)
    {
        argv[i+2] = (char*) process->argv[i];
    }

//    sel4utils_create_word_args(string_args, argv, argc, process->main.process_endpoint, process->init_remote_vaddr);

    /* spawn the process */

    process->stats.startTime = GetTime();
    vspace_t* mainVSpace = getMainVSpace();
    vka_t *mainVKA = getMainVKA();
    error = sel4utils_spawn_process_v(&process->native, mainVKA, mainVSpace,
                                      argc, argv, 1);
    ZF_LOGF_IF(error != 0, "Failed to start test process!");
}

void process_tear_down(Process* process)
{
    KernelTaskContext* env = getKernelTaskContext();

    Thread* elt = NULL;
    Thread* tmp = NULL;

    for(int i=0;i<process->argc;i++)
    {
        kfree(process->argv[i]);
    }
    kfree(process->argv);
    vka_t *mainVKA = getMainVKA();
    LL_FOREACH_SAFE(process->threads,elt,tmp) 
    {
        LL_DELETE(process->threads,elt);
        
        seL4_TCB_Suspend(elt->tcb.cptr);
        KLOG_DEBUG("Free TCB thread %p\n", elt);
        vka_free_object(mainVKA, &elt->tcb);
        KLOG_DEBUG("Did Free TCB thread %p\n", elt);

        if(elt->_base.replyCap != 0)
        {
            ThreadCleanupTimer(elt);
            if(elt->ipcBuffer_vaddr)
            {
                //vspace_unmap_pages(&process->native.vspace, elt->ipcBuffer_vaddr, 1, PAGE_BITS_4K, VSPACE_FREE);
            }
        }
        if(elt->stack && elt->stackSize)
        {
            KLOG_INFO("Remove Thread stack (size %zi)\n", elt->stackSize);
            vspace_free_sized_stack(&process->native.vspace, elt->stack, elt->stackSize);
            KLOG_DEBUG("Did remove thread stack\n");
        }
        kfree(elt);
    }

    if(process->main._base.replyCap)
    {
        ThreadCleanupTimer(&process->main);
    }

    /* unmap the env->init data frame */
    vspace_unmap_pages(&process->native.vspace, process->init_remote_vaddr, 1, PAGE_BITS_4K, VSPACE_FREE);
    vspace_unmap_pages(&process->native.vspace, process->init->mainIPCBuffer, 1, PAGE_BITS_4K, VSPACE_FREE);

    
    /* destroy the process */
    sel4utils_destroy_process(&process->native, mainVKA);   
}


void process_suspend(Process*p)
{
    seL4_TCB_Suspend(p->native.thread.tcb.cptr);
}
void process_resume(Process*p)
{
    seL4_TCB_Resume(p->native.thread.tcb.cptr);
}


void* process_new_pages(Process*p, seL4_CapRights_t rights, size_t numPages)
{
    void* pages = sel4utils_new_pages(&p->native.vspace, seL4_AllRights, numPages, PAGE_BITS_4K);
    if(pages)
    {
        p->stats.allocPages += numPages;
    }
    return pages;
}

void process_unmap_pages(Process*p, void *vaddr, size_t numPages)
{
    sel4utils_unmap_pages(&p->native.vspace, vaddr, numPages, PAGE_BITS_4K, VSPACE_FREE);
    p->stats.allocPages -= numPages;
}