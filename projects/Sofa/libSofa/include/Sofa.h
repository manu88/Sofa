#pragma once


typedef enum
{
    SyscallID_Unknown = 0,
    SyscallID_ThreadNew,
    SyscallID_ThreadExit,
    SyscallID_Exit,
    SyscallID_Sleep,

} SyscallIDs;



int SofaSleep(int ms);
