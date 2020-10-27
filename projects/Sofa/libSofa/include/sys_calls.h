#pragma once


typedef enum
{
    SofaSysCall_Unknown  = 0,
    SofaSysCall_InitProc = 1,
    SofaSysCall_Exit,    
    SofaSysCall_Write,
    SofaSysCall_Spawn,

    SofaSysCall_PPID,

    SofaSysCall_Debug
} SofaSysCall;