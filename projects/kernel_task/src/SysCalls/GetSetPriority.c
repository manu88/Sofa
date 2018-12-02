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

#include "../SysCalls.h"
#include <sys/resource.h>
#include <SysCallNum.h>
#include "../ProcessTable.h"

int handle_getpriority(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const int which = seL4_GetMR(1);
	const id_t who  = seL4_GetMR(2);
	// TODO implement me :)
	seL4_SetMR(0 , __SOFA_NR_getpriority);
	seL4_SetMR(1,-ENOSYS);
	seL4_Reply( message );
	return 0;
}

int handle_setpriority(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{ 
	const int which = seL4_GetMR(1);
	const id_t who  = seL4_GetMR(2);
	const int prio  = seL4_GetMR(3);
	
	int error = -ENOSYS;

	if( which == 0)// && who == 0)
	{
		Process* procToRenice = who == 0 ? senderProcess : ProcessTableGetByPID( who);
		assert(procToRenice);
		printf("Process %i asks to nice %i to %i\n", senderProcess->_pid , procToRenice->_pid , prio);
		// the sender nices itself
		error = ProcessSetPriority(context,procToRenice , prio);
	}

	seL4_SetMR(0 , __SOFA_NR_setpriority);
	seL4_SetMR(1,error);
        seL4_Reply( message );
        return 0;
}
