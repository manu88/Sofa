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
#include <SysCallNum.h>
#include <assert.h>
#include "../FileServer.h"
#include <fcntl.h>


int handle_stat(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{

        const int msgLen = seL4_MessageInfo_get_length(message);
        assert(msgLen > 2);
	int pathLen = seL4_GetMR(1);

	assert(pathLen <1024); // FIXME use a real string size!
	char path[1024] = {0};
	for(int i=0;i<pathLen ; i++)
	{
		path[i] = seL4_GetMR(2+i);
	}
	path[pathLen] = 0;
	printf("handle_stat for file '%s' \n" , path);


	Inode* node = FileServerGetINodeForPath( path , senderProcess->currentDir);
	int ret = 0;
	if (!node)
	{
		ret = -ENOENT;
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);

        seL4_SetMR(0, __SOFA_NR_stat );
        seL4_SetMR(1, ret);

        seL4_Reply( message );
	return 0;
}
