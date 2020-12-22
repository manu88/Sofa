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
#include <sys/socket.h>


int NetClientInit(void);

int NetClientSocket(int domain, int type, int protocol);
int NetClientClose(int handle);
int NetClientBind(int handle, const struct sockaddr *addr, socklen_t addrlen);

ssize_t NetClientRecvFrom(int handle, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t NetClientSendTo(int handle, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

