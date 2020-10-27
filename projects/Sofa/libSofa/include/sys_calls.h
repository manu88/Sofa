#pragma once


typedef enum
{
    SofaSysCall_Unknown  = 0,
    SofaSysCall_InitProc = 1,
    SofaSysCall_Exit     = 2,    
    SofaSysCall_Write    = 3,
    SofaSysCall_Sleep    = 4,

    SofaSysCall_Debug
} SofaSysCall;