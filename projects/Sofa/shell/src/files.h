#pragma once
#include <stddef.h>
#include <sys/types.h>

int VFSClientInit(void);

int VFSOpen(const char* path, int mode);
int VFSClose(int handle);
ssize_t VFSRead(int handle, char* data, size_t size);