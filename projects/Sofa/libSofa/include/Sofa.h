#pragma once


typedef enum
{
    SyscallID_Unknown = 0,

    SyscallID_ThreadNew,
    SyscallID_ThreadExit,

    SyscallID_Exit,

    SyscallID_Sleep,

    SyscallID_Spawn,

} SyscallIDs;



int SofaSleep(int ms);


int SofaSpawn(const char* path);