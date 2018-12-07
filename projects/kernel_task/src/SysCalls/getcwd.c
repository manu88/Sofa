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
#include "../FileServer.h"
#include <fcntl.h>


#define MAX_PATH_LEN 4096
int handle_getcwd(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	size_t bufferSize = seL4_GetMR(1);


	char str[MAX_PATH_LEN] = {0}; // muslc says so apparently
	ssize_t strSize =  InodeGetAbsolutePath( senderProcess->currentDir, str, MAX_PATH_LEN );

//	printf("cwd : '%s' \n" , str);

	size_t realMsgSize = strSize > bufferSize ? 0 : strSize;

	message = seL4_MessageInfo_new(0, 0, 0, 2 + realMsgSize);
	seL4_SetMR(0,__SOFA_NR_getcwd);

	if (realMsgSize == 0)
	{
		strSize = -ERANGE;
	}

	seL4_SetMR(1, strSize);

	for(int i=0; i< realMsgSize ; ++i)
        {
            seL4_SetMR(2 + i, str[i]);
        }

	seL4_Reply( message );
	return 0;
}


