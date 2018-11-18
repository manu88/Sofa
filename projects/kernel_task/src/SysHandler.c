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

#include "SysHandler.h"
#include <string.h>

typedef struct
{
	Inode sysRootNode; //   /sys/
	Inode sysKernNode; //   /sys/kernel/

	Inode sysKernLogNode //    /sys/kernel/log

} SysHandlerContext;

static SysHandlerContext _context;

int SysHandlerInit()
{
	memset(&_context , 0 , sizeof(SysHandlerContext ) );

	if (InodeInit(&_context.sysRootNode, INodeType_Folder, "sys") == 0)
    	{
        	return 0;
    	}

	if (InodeInit(&_context.sysKernNode, INodeType_Folder, "kernel") == 0)
        {
                return 0;
        }

	if (InodeAddChild(&_context.sysRootNode , &_context.sysKernNode) == 0)
	{
		return 0;
	}

	if (InodeInit(&_context.sysKernLogNode, INodeType_File, "log") == 0)
        {
                return 0;
        }

	if (InodeAddChild(&_context.sysKernNode , &_context.sysKernLogNode) == 0)
        {
                return 0;
        }

	return 1;
}



Inode* SysHandlerGetINode()
{
	return &_context.sysRootNode;
}
