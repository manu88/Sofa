#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>


int NetInit(void);

int NetSocket(int domain, int type, int protocol);
int NetBind(int handle, const struct sockaddr *addr, socklen_t addrlen);

ssize_t NetWrite(int handle, const char* data, size_t size);


ssize_t NetRecvFrom(int handle, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);