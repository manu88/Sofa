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

#pragma once


#include "Sofa.h"
#include <sel4utils/process.h>

#include "Bootstrap.h"
#include <data_struct/cvector.h>
#include "uthash.h"
#include "fs.h"
#include "Credentials.h"

typedef struct _Process Process;

typedef enum 
{
	ProcessState_Uninitialized = 0,
	ProcessState_Running,
	ProcessState_Suspended,

	ProcessState_Zombie	   = 100,

} ProcessState;


// a process list
typedef struct _ProcessListEntry ProcessListEntry;
struct _ProcessListEntry
{
    Process *process;
    LIST_ENTRY(_ProcessListEntry) entries;
};


// a waiter list
typedef struct _WaiterListEntry WaiterListEntry;

struct _WaiterListEntry
{
    Process *process;
    int reason;
    seL4_CPtr reply;
    KernelTaskContext* context;

    LIST_ENTRY(_WaiterListEntry) entries;
};


struct _inode;


typedef struct
{
    size_t numSysCalls;
    
} ProcessStats;

// a process
struct _Process
{
    Inode _processNode; // might stay first so that Inode can be cast to Process
    uint8_t _prio;
    sel4utils_process_t _process;
    ProcessState        _state;
    pid_t               _pid;

    char* cmdLine;
    uint64_t startTime;
    
    struct _Process*                       parent;
    LIST_HEAD(listhead, _ProcessListEntry) children;
    
    LIST_HEAD(listheadWaiters, _WaiterListEntry) waiters;


    struct _inode* currentDir;
    cvector_t fdNodes;
    
    
    Identity _identity;
    ProcessStats _procStats;
    
    UT_hash_handle hh; // main proc hash, i.e the Process table
};



int ProcessInit(Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int ProcessDeInit(Process * process ) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
Process* ProcessAlloc(void) SOFA_UNIT_TESTABLE;
int ProcessRelease(Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;


int ProcessGetNumChildren(const Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

// ask -1 for any child
Process* ProcessGetChildByPID( const Process* process , pid_t pid) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int ProcessStart(KernelTaskContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority ) NO_NULL_POINTERS;

int ProcessStop(KernelTaskContext* context,Process* process) NO_NULL_POINTERS;

static inline Process* ProcessGetParent( const Process* p)
{
	return p->parent;
}


int ProcessSetCmdLine(Process* process , const char* cmdline)NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

NO_NULL_POINTERS static inline void ProcessSetState(Process* process, ProcessState state)
{
	process->_state = state;
}

// returns 0 on sucess
// not intended to be public, but here for test purposes.
int ProcessSetParentShip(Process* parent , Process* child) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;


int ProcessSetPriority(KernelTaskContext* context,Process* process , uint8_t prio) NO_NULL_POINTERS;
int ProcessGetPriority(KernelTaskContext* context,Process* process , uint8_t *prio) NO_NULL_POINTERS;

size_t ProcessGetNumFDs( /*const*/ Process* process) NO_NULL_POINTERS;
struct _inode* ProcessGetNode( /*const*/ Process* process , int index) NO_NULL_POINTERS;

// return node id
int ProcessAppendNode( Process* process , struct _inode* node) NO_NULL_POINTERS;
int ProcessRemoveNode( Process* process , int fd) NO_NULL_POINTERS;

int ProcessDoCleanup(Process * process) NO_NULL_POINTERS;
int ProcessSignalStop(Process* process) NO_NULL_POINTERS;
//
int ProcessRegisterWaiter( Process* process , WaiterListEntry* waiter) NO_NULL_POINTERS;

int ProcessDoExit(KernelTaskContext* context,Process* process , int retCode)NO_NULL_POINTERS;
int ProcessSendSignal(KernelTaskContext* context,Process *process, int sig)NO_NULL_POINTERS;

int ProcessSetIdentity(Process* process , const Identity* identityToCopy) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int ProcessSetIdentityUID(Process* process , uid_t uid) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

typedef struct
{
	Process *process;
	seL4_CPtr reply;

} TimerContext;
