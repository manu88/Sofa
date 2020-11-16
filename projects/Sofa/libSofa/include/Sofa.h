#pragma once
#include <sys/types.h> // pid_t

typedef enum
{
    SyscallID_Unknown = 0,

    SyscallID_ThreadNew,
    SyscallID_ThreadExit,

    SyscallID_Exit,

    SyscallID_Sleep,

    SyscallID_Spawn,
    SyscallID_Wait,

    SyscallID_Read,

    SyscallID_Last // Not a real ID, just here to count ids
} SyscallID;



int SofaSleep(int ms);


int SofaSpawn(const char* path);

pid_t SofaWaitPid(pid_t pid, int *wstatus, int options);
pid_t SofaWait(int *wstatus);


int SofaReadChar(void);