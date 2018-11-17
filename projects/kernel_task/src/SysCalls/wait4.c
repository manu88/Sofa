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

#include "../Utils.h"
#include "../SysCalls.h"
#include "../ProcessDef.h"

int handle_wait4(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    const pid_t pidToWait = seL4_GetMR(1);
    const int options     = seL4_GetMR(2);

    // check if senderProcess has any children

    const int childrenCount = ProcessGetNumChildren( senderProcess);

    // no child to wait -> reply immediatly
    if(childrenCount == 0)
    {
	    seL4_SetMR(1, -ECHILD );
	    seL4_Reply( message );
	    return 0;
    }

    Process* processToWait = ProcessGetChildByPID(senderProcess , pidToWait);

    assert(processToWait);

    WaiterListEntry* waiter = malloc(sizeof(WaiterListEntry) );
    
    if(!waiter)
    {
	seL4_SetMR(1, -ENOMEM);
	seL4_Reply( message );
    }

    waiter->process = senderProcess;
    waiter->reply = get_free_slot(context); 
    waiter->context = context;
    int error = cnode_savecaller( context, waiter->reply );

    ProcessRegisterWaiter(processToWait, waiter);
    return 0;
}
