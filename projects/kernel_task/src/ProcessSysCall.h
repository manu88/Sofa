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

#pragma once

#include "Process.h"


#define Reply(i) seL4_Reply(i)

// The Syscall handler signature.
typedef void (*SysCallHandler)(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);


void processSysCall(Process *sender, seL4_MessageInfo_t info , seL4_Word sender_badge);


/* Handlers */



// in SysCalls/IO.c
void processWrite(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processRead(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);

// in SysCalls/Debug.c
void processDebug(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);

// in SysCalls/Proc.c
void processBootstrap(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processSpawn(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processKill(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processWait(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processExit(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);

// in SysCalls/Time.c
void processSleep(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processGetTime(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);

// in SysCalls/Misc.c
void processGetIDs(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processSetPriority(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processGetPriority(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processCapOp(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processResourceReq(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
// in SysCalls/Servers.c
void processRegisterServer(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
void processRegisterClient(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge);
