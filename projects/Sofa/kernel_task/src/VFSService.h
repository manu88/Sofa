/*
 * This file is part of the Sofa project
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
#include "IODevice.h"
#include <Thread.h>

typedef enum
{
    VFSSupported_Unknown = 0,
    VFSSupported_EXT2,

}VFSSupported;

int VFSServiceInit(void);


int VFSServiceStart(void);

int VFSServiceGetClientCWD(ThreadBase* sender, char** pat_ret);

//int VFSAddDEvice(IODevice *dev);

int PathIsAbsolute(const char* path);
char* ConcPath(const char* workingDir, const char* path);


