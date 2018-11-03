#pragma once


#include "Sofa.h"
#include <sel4utils/process.h>
#include "TimerWheel/TimersWheel.h"

#include "TimerWheel/queue.h"

#include "Bootstrap.h"
#include <data_struct/cvector.h>


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
    InitContext* context;

    LIST_ENTRY(_WaiterListEntry) entries;
};


struct _inode;


// a process
struct _Process
{
    sel4utils_process_t _process;
    ProcessState _state;
    pid_t               _pid;


    struct _Process *_parent;

    LIST_HEAD(listhead, _ProcessListEntry) children;
    
    LIST_HEAD(listheadWaiters, _WaiterListEntry) waiters;

    cvector_t fdNodes;
    //struct _inode *testNode;
//    seL4_CPtr reply;
    //Timer* _timer;
};



int ProcessInit(Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
int ProcessDeInit(Process * process ) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;
Process* ProcessAlloc(void) SOFA_UNIT_TESTABLE;
int ProcessRelease(Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;


int ProcessGetNumChildren(const Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

// ask -1 for any child
Process* ProcessGetChildByPID( const Process* process , pid_t pid) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int ProcessStart(InitContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority ) NO_NULL_POINTERS;

NO_NULL_POINTERS static inline void ProcessSetState(Process* process, ProcessState state)
{
	process->_state = state;
}

// returns 0 on sucess
// not intended to be public, but here for test purposes.
int ProcessSetParentShip(Process* parent , Process* child) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;


int ProcessSetPriority(InitContext* context,Process* process , uint8_t prio) NO_NULL_POINTERS;
int ProcessGetPriority(InitContext* context,Process* process , uint8_t *prio) NO_NULL_POINTERS;

struct _inode* ProcessGetNode( /*const*/ Process* process , int index) NO_NULL_POINTERS;

// return node id
int ProcessAppendNode( Process* process , struct _inode* node) NO_NULL_POINTERS;


int ProcessDoCleanup(Process * process) NO_NULL_POINTERS;
int ProcessSignalStop(Process* process) NO_NULL_POINTERS;
//
int ProcessRegisterWaiter( Process* process , WaiterListEntry* waiter) NO_NULL_POINTERS;


typedef struct
{
	Timer timer;
	Process *process;
	seL4_CPtr reply;

} TimerContext;
