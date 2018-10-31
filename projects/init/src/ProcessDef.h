#pragma once



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



struct _inode;


// a process
struct _Process
{
    sel4utils_process_t _process;
    ProcessState _state;
    pid_t               _pid;


    struct _Process *_parent;

    LIST_HEAD(listhead, _ProcessListEntry) children;


    cvector_t fdNodes;
    //struct _inode *testNode;
//    seL4_CPtr reply;
    //Timer* _timer;
};



int ProcessInit(Process* process);
Process* ProcessAlloc(void);
int ProcessRelease(Process* process);


int ProcessGetNumChildren(const Process* process);

int ProcessStart(InitContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority );

static inline void ProcessSetState(Process* process, ProcessState state)
{
	process->_state = state;
}



int ProcessSetPriority(InitContext* context,Process* process , uint8_t prio);
int ProcessGetPriority(InitContext* context,Process* process , uint8_t *prio);

struct _inode* ProcessGetNode( /*const*/ Process* process , int index);
int ProcessAppendNode( Process* process , struct _inode* node);

//


typedef struct
{
	Timer timer;
	Process *process;
	seL4_CPtr reply;

} TimerContext;
