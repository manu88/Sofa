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
#pragma once
#include <sys/types.h> // pid_t

typedef enum
{
    SyscallID_Unknown = 0,

    SyscallID_ThreadNew,
    SyscallID_ThreadExit,

    SyscallID_Exit,

    SyscallID_Sleep,

//    SyscallID_Spawn,
//    SyscallID_Wait,
//    SyscallID_Kill,
    SyscallID_mmap,
    SyscallID_munmap,

    SyscallID_Read,
    SyscallID_Write,
    SyscallID_PPID,
    SyscallID_Debug,

    SyscallID_RequestCap,

    SofaSysCall_RegisterService, // Register a 
    SofaSysCall_GetService,

    SofaSysCall_Reboot,

    SyscallID_Last // Not a real ID, just here to count ids
}SyscallID;

typedef enum
{
    SofaDebugCode_ListServices,
    SofaDebugCode_DumpSched,
}SofaDebugCode;


typedef enum
{
    SofaRequestCap_TCB,
    SofaRequestCap_NewThread,
    SofaRequestCap_IPCBuff,
    SofaRequestCap_Endpoint,
    SofaRequestCap_NewPage,
}SofaRequestCap;

typedef enum 
{
    VFSRequest_Register,
    VFSRequest_ListDir,
    VFSRequest_Open,
    VFSRequest_Close,
    VFSRequest_Read,
    VFSRequest_Write,
    VFSRequest_Seek,
    VFSRequest_Debug
    
} VFSRequest;


typedef enum 
{
    NetRequest_Register,
    NetRequest_Socket,
    NetRequest_Close,
    NetRequest_Bind,
    NetRequest_RecvFrom,
    NetRequest_SendTo,
    NetRequest_Debug,
} NetRequest;

int SFSleep(int ms);


pid_t SFGetPid(void);
pid_t SFGetPPid(void);

ssize_t SFRead(char* data, size_t dataSize);
ssize_t SFWrite(const char* data, size_t dataSize);

int SFPrintf(const char *format, ...) __attribute__((format(printf,1,2)));


// if returns -EAGAIN, it means that no endline was found in dataSize, BUT data was written.
// simply issue the call again to get the rest. 
ssize_t SFReadLine(char* data, size_t dataSize);

// Name server

ssize_t SFRegisterService(const char* name);
ssize_t SFGetService(const char* name);

typedef enum
{
    RebootMode_Shutdown
} RebootMode;
int SFShutdown(void);


long SFMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
long SFMunmap(void* addr, size_t length);


// temp/debug syscall
void SFDebug(SofaDebugCode code);