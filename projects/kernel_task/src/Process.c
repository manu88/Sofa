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

#include <stdio.h>
#include <string.h>
#include "ProcessDef.h"
#include <assert.h>
#include "Utils.h"
#include "Timer.h"
//#include "Timer.h"
#include "ProcessTable.h"
#include "SysCallNum.h"

/*
ssize_t ProcRead ( Inode * node, char*buffer  , size_t count);
//ssize_t ProcWrite ( Inode *node,  const char* buffer ,size_t count);

static FileOperations _processFileOps = 
{
	ProcRead , FileOperation_NoWrite , FileOperation_NoLseek
};
*/
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
    free(process); // FIXME to remove since Process can be allocated on the stack
    return 1;
}

int ProcessInit(Process* process)
{
    memset(process , 0 , sizeof(Process) );

    LIST_INIT(&process->children);
    LIST_INIT(&process->waiters);

    memset(&process->_processNode, 0, sizeof(Inode));
    process->_processNode.type = InodeType_Invalid;
    
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
    
    InodeRelease(&process->_processNode);

    if( process->cmdLine)
    {
        free(process->cmdLine);
    }


    return 1;
}





int ProcessStart(KernelTaskContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority )
{
    
#ifndef SOFA_TESTS_ONLY
    UNUSED int error = 0;

    sel4utils_process_config_t config = process_config_default_simple( &context->simple, imageName, priority);

    error = sel4utils_configure_process_custom( &process->_process , &context->vka , &context->vspace, config);

    ZF_LOGF_IFERR(error, "Failed to spawn a new thread.\n"
                  "\tsel4utils_configure_process expands an ELF file into our VSpace.\n"
                  "\tBe sure you've properly configured a VSpace manager using sel4utils_bootstrap_vspace_with_bootinfo.\n"
                  "\tBe sure you've passed the correct component name for the new thread!\n");



    

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

    process->startTime = GetCurrentTime();
    
    error = sel4utils_spawn_process_v(&process->_process , &context->vka , &context->vspace , argc, (char**) &argv , 1);
    ZF_LOGF_IFERR(error, "Failed to spawn and start the new thread.\n"
                  "\tVerify: the new thread is being executed in the root thread's VSpace.\n"
                  "\tIn this case, the CSpaces are different, but the VSpaces are the same.\n"
                  "\tDouble check your vspace_t argument.\n");


     ProcessSetState(process , ProcessState_Running);
     process->_prio = priority;

     error = ProcessSetParentShip(parent , process);
     assert(error == 0);
    
    

     return error;
#else
    return 0;
#endif
}

int ProcessStop(KernelTaskContext* context,Process* process)
{
#ifndef SOFA_TESTS_ONLY
    sel4utils_destroy_process( &process->_process, &context->vka);
#else
    return 0;
#endif
    return 1;
}

// returns 0 on sucess
int ProcessSetParentShip(Process* parent , Process* child)
{
     child->parent = parent;

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
		if(entry->process->parent != process)
		{
		    printf("Process %i supposed to be the parent of %i but found %i\n" , process->_pid , entry->process->_pid , entry->process->parent->_pid);
		    // sanity check child's parent must be the parent
		    assert(entry->process->parent == process);
		}
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

int ProcessSetPriority(KernelTaskContext* context,Process* process , uint8_t prio)
{
#ifndef SOFA_TESTS_ONLY
	int ret =  seL4_TCB_SetPriority( sel4utils_get_tcb(&process->_process.thread),
				     seL4_CapInitThreadTCB,//sel4utils_get_tcb(&process->_process.thread),
				     prio);

	if (ret == 0)
	{
	    process->_prio = prio;
	}

	return ret;
#else
    return 0;
#endif
}

int ProcessGetPriority(KernelTaskContext* context,Process* process , uint8_t *prio)
{
	*prio = process->_prio;
	return 1;
}

// FIXME should go const 
struct _inode* ProcessGetNode( Process* process , int index)
{
	return cvector_get(&process->fdNodes, index);
}

// FIXME should go const 
size_t ProcessGetNumFDs( /*const*/ Process* process)
{
	return cvector_count(&process->fdNodes);
}

int ProcessAppendNode( Process* process , struct _inode* node)
{
	cvector_add(&process->fdNodes ,node);
	return (int) cvector_count(&process->fdNodes) -1;
 
}

int ProcessRemoveNode( Process* process , int fd)
{
	cvector_delete(&process->fdNodes , fd);

	//FIXME add real retcode
	return 1;
}

int ProcessRegisterWaiter( Process* process , WaiterListEntry* waiter)
{
	LIST_INSERT_HEAD(&process->waiters, waiter, entries);
	return 1;
}

int ProcessSignalStop(Process* process)
{
#ifndef SOFA_TESTS_ONLY
    UNUSED int error = 0;

    WaiterListEntry* entry = NULL;
    WaiterListEntry* entry_temp = NULL;

    LIST_FOREACH_SAFE(entry, &process->waiters, entries ,entry_temp ) 
    {
        printf("Signal Stop : Got a process to notify!\n");
        assert(entry->context);
        assert(entry->process);
        
        // ensure process is still relevant
        if(ProcessTableContains(entry->process) )
        {
		assert(entry->reply);
                seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
        	seL4_SetMR(0, __SOFA_NR_wait4);
        	seL4_SetMR(1, process->_pid);

	        seL4_Send(entry->reply , tag);
        }
	
        cnode_delete(entry->context,entry->reply);
        LIST_REMOVE(entry , entries);
        free(entry);
    }

    return error;
#else
    return 0;
#endif
}



int ProcessSendSignal(KernelTaskContext* context,Process *process, int sig)
{
	printf("ProcessSendSignal %i to process %i \n", sig,  process->_pid);
	return ProcessDoExit(context, process, -sig);
}

static int _ProcessReapChildrenIfNeeded(Process* process)
{
    Process* initProcess = ProcessTableGetByPID(1);
    assert(initProcess);
    
    ProcessListEntry* entry = NULL;
    LIST_FOREACH(entry, &process->children, entries)
    {
	printf("ReapChild %i \n" , entry->process->_pid);

        ProcessSetParentShip(initProcess, entry->process);
    }
    return 1;
}

int ProcessDoExit(KernelTaskContext* context,Process* process , int retCode)
{
    printf("ProcessDoExit '%s' retCode %i\n" , process->cmdLine , retCode);

    ProcessSignalStop( process);
    ProcessDoCleanup( process);

    _ProcessReapChildrenIfNeeded(process);

    if(!ProcessTableRemove( process))
    {
        printf("Unable to remove process!\n");
    }

    ProcessStop(context , process);
    ProcessRelease(process);

    return 1;
}

int ProcessDoCleanup(Process * process)
{
	assert(process);

	return 1;
}


int ProcessSetCmdLine(Process* process , const char* cmdline)
{
    process->cmdLine = strdup(cmdline);
    /*
	assert(process->_processNode.type == INodeType_Folder);
	Inode* cmdLineNode =  InodeGetChildByName(&process->_processNode , "cmdline");
	assert(cmdLineNode);

	cmdLineNode->userData = strdup(cmdline);
	cmdLineNode->size = strlen(cmdline);
     */
 	return 1;
}

int ProcessSetIdentity(Process* process , const Identity* identityToCopy)
{
    if(memcpy(&process->_identity, identityToCopy, sizeof(Identity)) == &process->_identity)
    {
        return InodeSetIdentity(&process->_processNode, &process->_identity);
    }
    
    return 0;
}

int ProcessSetIdentityUID(Process* process , uid_t uid)
{
    process->_identity.uid = uid;
    process->_processNode._identity.uid = uid;
    return 1;
}


/*
ssize_t ProcRead ( Inode * node, char*buffer  , size_t count)
{
	Process* process = node->userData;
	assert(process);
	printf("Request to read process node pid %i\n" , process->_pid);
	return -1;
}
*/
