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

