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
#include <assert.h>
#include "../ProcessTable.h"

int handle_setresuid(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	assert(seL4_MessageInfo_get_length(message) == 4 );


	uid_t ruid = seL4_GetMR(1);
        uid_t euid = seL4_GetMR(2); // this is the one we care about for now
        uid_t suid = seL4_GetMR(3);

	long ret = -EPERM;

	if (euid >= senderProcess->_identity.uid)
	{
		ret = !ProcessSetIdentityUID(senderProcess , euid);
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);

        seL4_SetMR(0, __SOFA_NR_setresuid );
        seL4_SetMR(1, ret);

        seL4_Reply( message );

	return 0;
}
