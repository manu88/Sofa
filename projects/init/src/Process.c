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


#include <string.h>
#include "ProcessDef.h"
#include <SysCallNum.h>
#include <assert.h>
#include "Utils.h"
#include "ProcessTable.h"




Process* ProcessAlloc()
{
    Process* p = malloc(sizeof(Process));
    if(p)
    {
        ProcessInit(p);
        return p;
    }
    
    return NULL;
}

int ProcessRelease(Process* process)
{
    ProcessDeInit(process);
    free(process);
    return 1;
}

int ProcessInit(Process* process)
{
    memset(process , 0 , sizeof(Process) );

    LIST_INIT(&process->children);
    LIST_INIT(&process->waiters);

    cvector_init(&process->fdNodes);
    return 1;
}

int ProcessDeInit(Process * process )
{
    
    ProcessListEntry* entry = NULL;
    ProcessListEntry* entry_s = NULL;
    
    
    LIST_FOREACH_SAFE(entry, &process->children, entries, entry_s)
    {
        LIST_REMOVE(entry, entries);
        free(entry);
    }
    return 1;
}





int ProcessStart(InitContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority )
{
#ifndef __APPLE__
    UNUSED int error = 0;

    sel4utils_process_config_t config = process_config_default_simple( &context->simple, imageName, priority);

    error = sel4utils_configure_process_custom( &process->_process , &context->vka , &context->vspace, config);

    ZF_LOGF_IFERR(error, "Failed to spawn a new thread.\n"
                  "\tsel4utils_configure_process expands an ELF file into our VSpace.\n"
                  "\tBe sure you've properly configured a VSpace manager using sel4utils_bootstrap_vspace_with_bootinfo.\n"
                  "\tBe sure you've passed the correct component name for the new thread!\n");



    process->_pid = ProcessTableGetNextPid();

    seL4_CPtr process_ep_cap = 0;

    process_ep_cap = sel4utils_mint_cap_to_process(&process->_process, ep_cap_path,
                                               seL4_AllRights, process->_pid);

    ZF_LOGF_IF(process_ep_cap == 0, "Failed to mint a badged copy of the IPC endpoint into the new thread's CSpace.\n"
               "\tsel4utils_mint_cap_to_process takes a cspacepath_t: double check what you passed.\n");


    process_config_fault_cptr(config, ep_cap_path.capPtr);

    seL4_Word argc = 1;
    char string_args[argc][WORD_STRING_SIZE];
    char* argv[argc];
    sel4utils_create_word_args(string_args, argv, argc ,process_ep_cap);


    printf("init : Start child \n");
    error = sel4utils_spawn_process_v(&process->_process , &context->vka , &context->vspace , argc, (char**) &argv , 1);
    ZF_LOGF_IFERR(error, "Failed to spawn and start the new thread.\n"
                  "\tVerify: the new thread is being executed in the root thread's VSpace.\n"
                  "\tIn this case, the CSpaces are different, but the VSpaces are the same.\n"
                  "\tDouble check your vspace_t argument.\n");

     printf("init : Did start child pid %i\n" , process->_pid);

     ProcessSetState(process , ProcessState_Running);

     error = ProcessSetParentShip(parent , process);
     assert(error == 0);

     return error;
#else
    return 0;
#endif
}

int ProcessStop(InitContext* context,Process* process)
{
#ifndef __APPLE__
    sel4utils_destroy_process( &process->_process, &context->vka);
#else
    return 0;
#endif
    return 1;
}

// returns 0 on sucess
int ProcessSetParentShip(Process* parent , Process* child)
{
     child->_parent = parent;

     ProcessListEntry* entry = (ProcessListEntry*) malloc(sizeof(ProcessListEntry)) ;
     assert(entry);
     entry->process = child;

     LIST_INSERT_HEAD(&parent->children, entry, entries);

     return 0;
}


int ProcessGetNumChildren(const Process* process)
{
	ProcessListEntry* entry = NULL;
	int count = 0;
	LIST_FOREACH(entry, &process->children, entries) 
	{
		// sanity check child's parent must be the parent
		assert(entry->process->_parent == process);
		count++;
	}
	return count;
}

Process* ProcessGetChildByPID( const Process* process , pid_t pid)
{
	ProcessListEntry* entry = NULL;
	LIST_FOREACH(entry, &process->children, entries) 
        {
		if (entry->process->_pid == pid || pid == -1)
		{	
			return entry->process;
		}
	}
	return NULL;
}

int ProcessSetPriority(InitContext* context,Process* process , uint8_t prio)
{
#ifndef __APPLE__
	return seL4_TCB_SetPriority( sel4utils_get_tcb(&process->_process.thread),
				     seL4_CapInitThreadTCB,//sel4utils_get_tcb(&process->_process.thread),
				     prio);
#else
    return 0;
#endif
}

int ProcessGetPriority(InitContext* context,Process* process , uint8_t *prio)
{
	return -1;
}

// FIXME should go const 
struct _inode* ProcessGetNode( Process* process , int index)
{
	return cvector_get(&process->fdNodes, index);
}

int ProcessAppendNode( Process* process , struct _inode* node)
{
	cvector_add(&process->fdNodes ,node);
	return (int) cvector_count(&process->fdNodes) -1;
 
}

int ProcessRegisterWaiter( Process* process , WaiterListEntry* waiter)
{
	LIST_INSERT_HEAD(&process->waiters, waiter, entries);
	return 1;
}

int ProcessSignalStop(Process* process)
{
#ifndef __APPLE__
    UNUSED int error = 0;

    WaiterListEntry* entry = NULL;
    WaiterListEntry* entry_temp = NULL;

    LIST_FOREACH_SAFE(entry, &process->waiters, entries ,entry_temp ) 
    {
	printf("Signal Stop : Got a process to notify!\n");
	assert(entry->context);
	assert(entry->process);

	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0, __SOFA_NR_wait4);
	seL4_SetMR(1, process->_pid);
	
         seL4_Send(entry->reply , tag); 
	cnode_delete(entry->context,entry->reply);

	LIST_REMOVE(entry , entries);
	free(entry);
    }

    return error;
#else
    return 0;
#endif
}


int ProcessDoCleanup(Process * process)
{
	assert(process);

	return 1;
}
