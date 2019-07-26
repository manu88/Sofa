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

typedef enum
{
    SofaCap_None         = 0,        // The proc can't use system resources.
    
    SofaCap_Nice         = 1 << 0,  // can nice up itself
    SofaCap_Kill         = 1 << 1,  // really kill ANY proc?
    SofaCap_Spawn        = 1 << 2,  // spawned procs can be killed, even is SofaCap_Kill is not present
    SofaCap_CreateServer = 1 << 3,
    

} SofaCapabilities;

/*
 A process can't list its caps.
 A process can ALWAYS drops caps.
 A process can ALWAYS asks to gain a cap, but must be prepared to be refused. This is the only time a process can known it has a cap.
 A process can Silently loose a cap. Aside from a side effect like not be able to open a file for example, the process has no ways to know it.
 */


void CapDrop( SofaCapabilities cap);
int  CapAcquire( SofaCapabilities cap);
