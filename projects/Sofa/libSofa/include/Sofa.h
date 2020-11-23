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
    SyscallID_Kill,

    SyscallID_Read,
    SyscallID_Write,
    SyscallID_PPID,
    SyscallID_Debug,

    SyscallID_RequestCap,

    SyscallID_Last // Not a real ID, just here to count ids
}SyscallID;

typedef enum
{
    SofaDebugCode_ListProcesses,
    SofaDebugCode_DumpSched,
}SofaDebugCode;


typedef enum
{
    SofaRequestCap_TCB,
    SofaRequestCap_MAP,
}SofaRequestCap;

int SofaSleep(int ms);


int SofaSpawn(const char* path);

pid_t SofaGetPid(void);
pid_t SofaGetPPid(void);

pid_t SofaWaitPid(pid_t pid, int *wstatus, int options);
pid_t SofaWait(int *wstatus);

int SofaKill(pid_t pid, int sig);

ssize_t SofaRead(char* data, size_t dataSize);
ssize_t SofaWrite(const char* data, size_t dataSize);

int SofaPrintf(const char *format, ...);


// if returns -EAGAIN, it means that no endline was found in dataSize, BUT data was written.
// simply issue the call again to get the rest. 
ssize_t SofaReadLine(char* data, size_t dataSize);



// temp/debug syscall

void SofaDebug(SofaDebugCode code);