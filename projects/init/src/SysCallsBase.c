#include "SysCalls.h"
#include "Utils.h"
#include "ProcessTable.h"
#include "MainLoop.h" // UpdateTimeout
 
int handle_getpid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    seL4_SetMR(1, senderProcess->_pid);
    seL4_Reply( message );

    return 0;
}

int handle_getppid(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    seL4_SetMR(1, senderProcess->_parent->_pid );
    seL4_Reply( message );
    return 0;
}

int handle_exit(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        printf("Got exit from %i\n", senderProcess->_pid);

    if(!ProcessTableRemove( senderProcess))
    {
        printf("Unable to remove process!\n");
    }

    sel4utils_destroy_process( &senderProcess->_process, &context->vka);
    ProcessRelease(senderProcess);

    printf("Init : Got %i processes \n" , ProcessTableGetCount() );
    return 0;
}

int handle_kill(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        seL4_Word pidToKill = seL4_GetMR(1);
    seL4_Word sigToSend = seL4_GetMR(2);
    
    printf("Received a request from %i to kill process %li with signal %li\n",senderProcess->_pid , pidToKill , sigToSend);
    seL4_SetMR(1, -ENOSYS ); // error for now
    seL4_Reply( message );

    return 0;
}


int handle_nanosleep(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    seL4_Word millisToWait = seL4_GetMR(1);
    //printf("Process %i request to sleep %li ms\n" , senderProcess->_pid , millisToWait);

    // save the caller

    senderProcess->reply = get_free_slot(context);

    int error = cnode_savecaller( context, senderProcess->reply );

    assert(error == 0);

    Timer* timer = TimerAlloc( senderProcess,1/*oneShot*/);
    assert(timer);
    assert(TimerWheelAddTimer(&context->timersWheel , timer , millisToWait));
    UpdateTimeout(context,millisToWait * NS_IN_MS);

    return 0;
}

int handle_wait4(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        const pid_t pidToWait = seL4_GetMR(1);
    const int options     = seL4_GetMR(2);

    seL4_SetMR(1,-ENOSYS );
    seL4_Reply( message );
        return 0;
}


int handle_execve(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    const int msgLen = seL4_MessageInfo_get_length(message);
    assert(msgLen > 0);
    char* filename = malloc(sizeof(char)*msgLen );/* msglen minus header + 1 byte for NULL byte*/
    assert(filename);
    
    for(int i=0;i<msgLen-1;++i)
    {
        filename[i] =  (char) seL4_GetMR(1+i);
    }

    filename[msgLen] = '0';

    printf("Init : Execve size %i filename '%s'\n", msgLen , filename);

    /**/
    Process *newProcess = ProcessAlloc();
    assert(newProcess);
        
        int error = startProcess(context,  newProcess,filename, context->ep_cap_path ,senderProcess, APP_PRIORITY);
    
    if (error == 0)
    {
        error = !ProcessTableAppend(newProcess);
        assert(error == 0);
    }

    /**/
    free(filename);

    seL4_SetMR(1, newProcess->_pid/*  -ENOSYS*/ ); // return code is the new pid
    seL4_Reply( message );
        return 0;
}
