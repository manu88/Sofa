/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include "ProcessSysCall.h"



// order MUST Match SysCallID
static SysCallHandler _sysHandler[SysCall_Last] =
{
    NULL, // 0 is UNUSED for now
    processBootstrap,
    processGetIDs,
    processWrite,
    processSleep,
    processRead,
    processDebug,
    processSpawn,
    processKill,
    processWait,
    processExit,
    processGetTime,
    processSetPriority,
    processGetPriority,
    processCapOp,
    //processResourceReq,
    //processRegisterServer,
    //processRegisterClient,
};


/*
static inline void Reply(seL4_MessageInfo_t info)
{
    printf("Reply syscall %li\n" , seL4_GetMR(0));
    seL4_Reply(info);
}
 */

void processSysCall(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    assert(sender);
    const SysCallID sysCallID = seL4_GetMR(0);
    
    _sysHandler[sysCallID](sender,info ,sender_badge);
    
    sender->stats.numSysCalls++;
}

