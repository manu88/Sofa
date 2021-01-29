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
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

int VFSClientInit(void);

int VFSClientOpen(const char* path, int mode);
int VFSClientClose(int handle);

ssize_t VFSClientRead(int handle, char* data, size_t size);
ssize_t VFSClientWrite(int handle, const char* data, size_t size);

off_t VFSClientSeek(int handle, off_t offset, int whence);

int VFSClientStat(const char *pathname, struct stat *statbuf);

char* VFSClientGetCWD(char *buf, size_t size);
int VFSClientChDir(const char* path);


int Printf(const char *format, ...) __attribute__((format(printf,1,2)));