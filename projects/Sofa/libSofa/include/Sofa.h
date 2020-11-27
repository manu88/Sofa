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
    SofaRequestCap_IPCBuff,
}SofaRequestCap;

int SFSleep(int ms);


int SFSpawn(const char* path);

pid_t SFGetPid(void);
pid_t SFGetPPid(void);

pid_t SFWaitPid(pid_t pid, int *wstatus, int options);
pid_t SFWait(int *wstatus);

int SFKill(pid_t pid, int sig);

ssize_t SFRead(char* data, size_t dataSize);
ssize_t SFWrite(const char* data, size_t dataSize);

int SFPrintf(const char *format, ...);


// if returns -EAGAIN, it means that no endline was found in dataSize, BUT data was written.
// simply issue the call again to get the rest. 
ssize_t SFReadLine(char* data, size_t dataSize);



// temp/debug syscall

void SFDebug(SofaDebugCode code);