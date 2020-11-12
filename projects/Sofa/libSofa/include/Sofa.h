#pragma once


typedef enum
{
    SyscallID_Unknown = 0,
    SyscallID_NewThread,
    SyscallID_Exit,
    SyscallID_Sleep,

} SyscallIDs;



int SofaSleep(int ms);
int SofaSleep2(int ep, int ms);