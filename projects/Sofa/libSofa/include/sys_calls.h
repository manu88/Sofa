#pragma once


typedef enum
{
    SofaSysCall_Unknown = 0,
    SofaSysCall_InitProc = 1,
    SofaSysCall_Write = 2,
    SofaSysCall_Sleep = 3,

    SofaSysCall_Debug
} SofaSysCall;