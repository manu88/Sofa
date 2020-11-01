#pragma once


typedef enum
{
    SofaSysCall_Unknown  = 0,
    SofaSysCall_InitProc = 1,
    SofaSysCall_Exit,    
    SofaSysCall_Write,
    SofaSysCall_Spawn,

    SofaSysCall_PPID,
    SofaSysCall_Wait,


    SofaSysCall_RegisterService,
    SofaSysCall_GetService,


    SofaSysCall_Debug,
    SofaSysCall_TestCap

} SofaSysCall;


typedef enum
{
    DebugCode_DumpScheduler,
    DebugCode_ListProcesses,
    DebugCode_ListIPCServers
} DebugCode;