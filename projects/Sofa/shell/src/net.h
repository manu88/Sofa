#pragma once
#include <stddef.h>
#include <sys/types.h>


int NetInit(void);

int NetSocket(int domain, int type, int protocol);
int NetBind(int familly, int protoc, int port);

ssize_t NetRead(int handle, char* data, size_t size);
ssize_t NetWrite(int handle, const char* data, size_t size);